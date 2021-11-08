#include "vehicle.h"

#include <Arduino.h>
#include "binary.h"
#include "connection.h"
#include "dash.h"
#include "debug.h"


inline uint8_t clampTemp(uint8_t temp) {
    if (temp < 60) {
        temp = 60;
    } else if (temp > 90) {
        temp = 90;
    }
    return temp;
}

VehicleClimate::VehicleClimate() {
    can_ = nullptr;
    dash_ = nullptr;
    init_complete_ = false;

    // Send keepalive every 100ms during handshake.
    last_write_ = 0;
    keepalive_interval_ = 100;
    write_count_ = 0;

    // Set control frames to send handshake.
    memset(frame540_, 0, 8);
    memset(frame541_, 0, 8);
    frame540_[0] = 0x80;
    frame541_[0] = 0x80;
    frame54x_changed_ = true;
}

void VehicleClimate::connect(Connection* can, DashController* dash) {
    can_ = can;
    dash_ = dash;
}

bool VehicleClimate::climateOnline() const {
    return frame540_[0] == 0x60;
}

void VehicleClimate::climateClickOff() {
    if (!toggleFunction(frame540_, 6, 7)) {
        return;
    }

    active_ = false;
    auto_ = false;
    ac_ = false;
    recirculate_ = false;
    front_defrost_ = false;
    updateDash();
}

void VehicleClimate::climateClickAuto() {
    if (!toggleFunction(frame540_, 6, 5)) {
        return;
    }

    auto_ = !auto_;
    active_ = true;
    if (auto_) {
        ac_ = true;
        // TODO: check this transition
        recirculate_ = false;
        front_defrost_ = false;
        // TODO: check this transition
        if (fan_speed_ == 0) {
            fan_speed_ = 3;
        }
        // TODO: check this transition
        switch (mode_) {
            case MODE_WINDSHIELD:
            case MODE_FEET_WINDSHIELD:
                mode_ = MODE_AUTO_FACE;
                break;
        }
    }
    updateDash();
}

void VehicleClimate::climateClickAc() {
    if (!toggleFunction(frame540_, 5, 3) || !active_) {
        return;
    }
    ac_ = !ac_;
    updateDash();
}

void VehicleClimate::climateClickDual() {
    if (!toggleFunction(frame540_, 6, 3) || !active_) {
        return;
    }
    dual_ = !dual_;
    if (!dual_) {
        passenger_temp_ = driver_temp_;
    }
    updateDash();
}

void VehicleClimate::climateClickRecirculate() {
    if (!toggleFunction(frame541_, 1, 6) || !active_) {
        return;
    }
    auto_ = false;
    if (recirculate_) {
        recirculate_ = false;
    } else if (!front_defrost_) {
        switch (mode_) {
            case MODE_FACE:
            case MODE_FACE_FEET:
            case MODE_AUTO_FACE:
            case MODE_AUTO_FACE_FEET:
                recirculate_ = true;
                break;
        }
    }
    updateDash();
}

void VehicleClimate::climateClickMode() {
    // TODO: check if mode enables hvac
    // TODO: check how hitting mode transitions out of defrost
    if (!toggleFunction(frame540_, 6, 0) || !active_) {
        return;
    }
    auto_ = false;
    front_defrost_ = false;

    switch (mode_) {
        case MODE_FACE:
        case MODE_AUTO_FACE:
            mode_ = MODE_FACE_FEET;
            break;
        case MODE_FACE_FEET:
        case MODE_AUTO_FACE_FEET:
            mode_ = MODE_FEET;
            recirculate_ = false;
            break;
        case MODE_FEET:
        case MODE_AUTO_FEET:
            mode_ = MODE_FEET_WINDSHIELD;
            recirculate_ = false;
            break;
        case MODE_FEET_WINDSHIELD:
        default:
            mode_ = MODE_FACE;
            break;
    }
    updateDash();
}

void VehicleClimate::climateClickFrontDefrost() {
    if (!toggleFunction(frame540_, 6, 1)) {
        return;
    }

    front_defrost_ = !front_defrost_;
    if (front_defrost_) {
        active_ = true;
        ac_ = true;
        dual_ = false;
        recirculate_ = false;
        passenger_temp_ = driver_temp_;
    }
    updateDash();
}

void VehicleClimate::climateClickRearDefrost() {
    // TODO: determine rear defrost control signal
    if (!climateOnline()) {
        return;
    }
    rear_defrost_ = !rear_defrost_;
    updateDash();
}

void VehicleClimate::climateClickFanSpeedUp() {
    if (!toggleFunction(frame541_, 0, 5)) {
        return;
    }

    auto_ = false;
    if (!active_) {
        active_ = true;
        // TODO: check this transition
        ac_ = true;
    }
    if (fan_speed_ < 7) {
        fan_speed_++;
    }
    updateDash();
}

void VehicleClimate::climateClickFanSpeedDown() {
    if (!toggleFunction(frame541_, 0, 4)) {
        return;
    }

    auto_ = false;
    // TODO: check this transition
    if (!active_) {
        active_ = true;
        // TODO: check this transition
        ac_ = true;
    }
    if (fan_speed_ > 1) {
        fan_speed_--;
    }
    updateDash();
}

void VehicleClimate::climateClickDriverTempUp() {
    if (toggleTemperature()) {
        frame540_[3]++;
    }
    driver_temp_ = clampTemp(driver_temp_++);
    if (!dual_) {
        passenger_temp_ = driver_temp_;
    }
    updateDash();
}

void VehicleClimate::climateClickDriverTempDown() {
    if (toggleTemperature()) {
        frame540_[3]--;
    }
    driver_temp_ = clampTemp(driver_temp_--);
    if (!dual_) {
        passenger_temp_ = driver_temp_;
    }
    updateDash();
}

void VehicleClimate::climateClickPassengerTempUp() {
    if (toggleTemperature()) {
        frame540_[4]++;
    }
    dual_ = true;
    passenger_temp_ = clampTemp(passenger_temp_++);
    updateDash();
}

void VehicleClimate::climateClickPassengerTempDown() {
    if (toggleTemperature()) {
        frame540_[4]--;
    }
    dual_ = true;
    passenger_temp_ = clampTemp(passenger_temp_--);
    updateDash();
}

void VehicleClimate::push() {
    // Send control frames at least once every 200ms to keep the A/C Auto Amp alive.
    if (frame54x_changed_ || millis() - last_write_ >= keepalive_interval_) {
        D(if (frame54x_changed_) {
          INFO_MSG_FRAME("vehicle: send ", 0x540, 8, frame540_);
        })
        if (can_ && !can_->write(0x540, 8, frame540_)) {
            ERROR_MSG("vehicle: failed to send frame 0x540");
            return;
        }
        D(if (frame54x_changed_) {
          INFO_MSG_FRAME("vehicle: send ", 0x541, 8, frame541_);
        })
        if (can_ && !can_->write(0x541, 8, frame541_)) {
            ERROR_MSG("vehicle: failed to send frame 0x541");
            return;
        }

        frame54x_changed_ = false;
        last_write_ = millis();
        write_count_++;

        if (!init_complete_ && write_count_ >= 4) {
            frame540_[0] = 0x60;
            frame540_[1] = 0x40;
            frame540_[6] = 0x04;
            frame541_[0] = 0x00;
            init_complete_ = true;
            keepalive_interval_ = 200;
        }
    }
}

bool VehicleClimate::toggleFunction(byte* frame, uint8_t offset, uint8_t bit) {
    if (!climateOnline()) {
        return false;
    }
    toggleBit(frame, offset, bit);
    frame54x_changed_ = true;
    return true;
}

bool VehicleClimate::toggleTemperature() {
    return toggleFunction(frame540_, 5, 5);
}

void VehicleClimate::receive(uint32_t id, uint8_t len, byte* data) {
    switch(id) {
        case 0x54A:
            receive54A(len, data);
            break;
        case 0x54B:
            receive54B(len, data);
            break;
    }
}

void VehicleClimate::receive54A(uint8_t len, byte* data) {
    if (len != 8) {
        ERROR_MSG_VAL("vehicle: frame 0x54A has invalid length: 8 != ", len);
        return;
    }
    INFO_MSG_FRAME("vehicle: receive ", 0x54A, 8, data);

    uint8_t temp = data[4];
    if (temp != 0) {
        driver_temp_ = clampTemp(temp);
    }
    temp = data[5];
    if (temp != 0) {
        passenger_temp_ = clampTemp(temp);
    }

    updateDash();
}

void VehicleClimate::receive54B(uint8_t len, byte* data) {
    if (len != 8) {
        ERROR_MSG_VAL("vehicle: frame 0x54B has invalid length: 8 != ", len);
        return;
    }
    INFO_MSG_FRAME("vehicle: receive ", 0x54B, 8, data);

    active_ = !getBit(data, 0, 5);
    auto_ = getBit(data, 0, 0);
    ac_ = getBit(data, 0, 3);
    dual_ = getBit(data, 3, 7);
    recirculate_ = getBit(data, 3, 4);
    uint8_t fan_speed = (data[2] + 1) / 2;
    if (fan_speed != 0) {
        fan_speed_ = fan_speed;
    }

    front_defrost_ = data[1] == MODE_WINDSHIELD;
    if (!front_defrost_) {
        mode_ = data[1];
    }

    updateDash();
}

void VehicleClimate::updateDash() {
    dash_->setClimateActive(active_);
    dash_->setClimateAuto(auto_);
    dash_->setClimateAc(ac_);
    dash_->setClimateDual(dual_);
    dash_->setClimateRearDefrost(rear_defrost_);

    if (active_) {
        dash_->setClimateFanSpeed(fan_speed_);
        dash_->setClimateDriverTemp(driver_temp_);
        dash_->setClimatePassengerTemp(passenger_temp_);
    } else {
        dash_->setClimateFanSpeed(0);
        dash_->setClimateDriverTemp(0);
        dash_->setClimatePassengerTemp(0);
    }

    uint8_t mode = mode_;
    if (front_defrost_) {
        mode = MODE_WINDSHIELD;
    }
    switch (mode) {
        case MODE_FACE:
        case MODE_AUTO_FACE:
            dash_->setClimateFace(true);
            dash_->setClimateFeet(false);
            dash_->setClimateFrontDefrost(false);
            break;
        case MODE_FACE_FEET:
        case MODE_AUTO_FACE_FEET:
            dash_->setClimateFace(true);
            dash_->setClimateFeet(true);
            dash_->setClimateFrontDefrost(false);
            break;
        case MODE_FEET:
        case MODE_AUTO_FEET:
            dash_->setClimateFace(false);
            dash_->setClimateFeet(true);
            dash_->setClimateFrontDefrost(false);
            break;
        case MODE_FEET_WINDSHIELD:
            dash_->setClimateFace(false);
            dash_->setClimateFeet(true);
            dash_->setClimateFrontDefrost(true);
            break;
        case MODE_WINDSHIELD:
            dash_->setClimateFace(false);
            dash_->setClimateFeet(false);
            dash_->setClimateFrontDefrost(true);
            break;
        default:
            dash_->setClimateFace(false);
            dash_->setClimateFeet(false);
            dash_->setClimateFrontDefrost(false);
            break;
    }
}
