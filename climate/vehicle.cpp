#include "vehicle.h"

#include <Arduino.h>
#include "binary.h"
#include "connection.h"
#include "dash.h"
#include "debug.h"

static const uint8_t kDriverTempControlOffset = 0xB8;
static const uint8_t kPassengerTempControlOffset = 0xCC;

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

void VehicleClimate::deactivateClimate() {
    if (!toggle(frame540_, 6, 7)) {
        return;
    }
    active_ = false;
    updateDash();
}

void VehicleClimate::toggleClimateAuto() {
    if (!toggle(frame540_, 6, 5)) {
        return;
    }
    auto_ = !auto_;
    updateDash();
}

void VehicleClimate::toggleClimateAc() {
    if (!toggle(frame540_, 5, 3) || !active_) {
        return;
    }
    ac_ = !ac_;
    auto_ = false;
    updateDash();
}

void VehicleClimate::toggleClimateDual() {
    if (!toggle(frame540_, 6, 3) || !active_) {
        return;
    }
    dual_ = !dual_;
    if (!dual_) {
        setClimatePassengerTemp(driver_temp_);
    }
    updateDash();
}

void VehicleClimate::toggleClimateRecirculate() {
    if (!toggle(frame541_, 1, 6) || !active_) {
        return;
    }
    auto_ = false;
    if (recirculate_) {
        recirculate_ = false;
    } else if (mode_ != MODE_WINDSHIELD && mode_ != MODE_FEET_WINDSHIELD) {
        recirculate_ = !recirculate_;
    }
    updateDash();
}

void VehicleClimate::cycleClimateMode() {
    if (!toggle(frame540_, 6, 0)) {
        return;
    }
    active_ = true;
    auto_ = false;
    switch (mode_) {
        case MODE_FACE:
        case MODE_AUTO_FACE:
            mode_ = MODE_FACE_FEET;
            break;
        case MODE_FACE_FEET:
        case MODE_AUTO_FACE_FEET:
            mode_ = MODE_FEET;
            break;
        case MODE_FEET:
        case MODE_AUTO_FEET:
            mode_ = MODE_FEET_WINDSHIELD;
            break;
        case MODE_FEET_WINDSHIELD:
            mode_ = MODE_WINDSHIELD;
            break;
        default:
        case MODE_WINDSHIELD:
            mode_ = MODE_FACE;
            break;
    }
    updateDash();
}

void VehicleClimate::toggleClimateFrontDefrost() {
    if (!toggle(frame540_, 6, 1)) {
        return;
    }
    if (mode_ == MODE_WINDSHIELD|| mode_ == MODE_FEET_WINDSHIELD) {
        auto_ = true; 

        if (mode_ == MODE_FEET_WINDSHIELD) {
            mode_ = MODE_FEET;
        } else {
            mode_ = MODE_FACE;
        }
    } else {
        active_ = true;
        auto_ = false;
        dual_ = true;
        passenger_temp_ = driver_temp_;

        switch (mode_) {
            case MODE_FEET:
            case MODE_AUTO_FEET:
            case MODE_FACE_FEET:
            case MODE_AUTO_FACE_FEET:
                mode_ = MODE_FEET_WINDSHIELD;
                break;
            default:
                mode_ = MODE_WINDSHIELD;
                break;
        }
    }
    updateDash();
}

void VehicleClimate::toggleClimateRearDefrost() {
    // TODO: determine rear defrost control signal
    if (climateOnline()) {
        INFO_MSG("vehicle: climate: toggle rear defrost not implemented");
    }
}

void VehicleClimate::increaseClimateFanSpeed() {
    if (!toggle(frame541_, 0, 5)) {
        return;
    }
    active_ = true;
    auto_ = false;
    if (fan_speed_ < 7) {
        fan_speed_++;
    }
}

void VehicleClimate::decreaseClimateFanSpeed() {
    if (!toggle(frame541_, 0, 4)) {
        return;
    }
    if (!active_) {
        return;
    }
    auto_ = false;
    if (fan_speed_ > 1) {
        fan_speed_--;
    }
}

void VehicleClimate::increaseClimateDriverTemp(uint8_t value) {
    setClimateDriverTemp(driver_temp_ + value);
}

void VehicleClimate::decreaseClimateDriverTemp(uint8_t value) {
    setClimateDriverTemp(driver_temp_ - value);
}

void VehicleClimate::setClimateDriverTemp(uint8_t temp) {
    if (!climateOnline() || !active_) {
        return;
    }

    if (temp < 60 ) {
        temp = 60;
    }
    if (temp > 90) {
        temp = 90;
    }

    toggleSetTemperatureBit();
    setDriverTempByte(temp);
    if (!dual_) {
        setPassengerTempByte(temp);
    }

    updateDash();
}

void VehicleClimate::increaseClimatePassengerTemp(uint8_t value) {
    setClimatePassengerTemp(passenger_temp_ + value);
}

void VehicleClimate::decreaseClimatePassengerTemp(uint8_t value) {
    setClimatePassengerTemp(passenger_temp_ - value);
}

void VehicleClimate::setClimatePassengerTemp(uint8_t temp) {
    if (!climateOnline() || !active_) {
        return;
    }

    if (temp < 60 ) {
        temp = 60;
    }
    if (temp > 90) {
        temp = 90;
    }

    dual_ = true;

    toggleSetTemperatureBit();
    setPassengerTempByte(temp);

    updateDash();
}

void VehicleClimate::setDriverTempByte(uint8_t temp) {
    driver_temp_ = temp;
    frame540_[3] = driver_temp_ + kDriverTempControlOffset;
}

void VehicleClimate::setPassengerTempByte(uint8_t temp) {
    passenger_temp_ = temp;
    frame540_[4] = passenger_temp_ + kPassengerTempControlOffset;
}

void VehicleClimate::push() {
    if (can_ == nullptr) {
        return;
    }

    // Send control frames at least every 200ms to keep the A/C Auto Amp alive.
    if (frame54x_changed_ || millis() - last_write_ >= keepalive_interval_) {
        D(if (frame54x_changed_) {
          INFO_MSG_FRAME("vehicle: send ", 0x540, 8, frame540_);
        })
        if (!can_->write(0x540, 8, frame540_)) {
            ERROR_MSG("vehicle: failed to send frame 0x540");
            return;
        }
        D(if (frame54x_changed_) {
          INFO_MSG_FRAME("vehicle: send ", 0x541, 8, frame541_);
        })
        if (!can_->write(0x541, 8, frame541_)) {
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

bool VehicleClimate::toggle(byte* frame, uint8_t offset, uint8_t bit) {
    if (!climateOnline()) {
        return false;
    }
    toggleBit(frame, offset, bit);
    frame54x_changed_ = true;
    return true;
}

void VehicleClimate::toggleSetTemperatureBit() {
    toggleBit(frame540_, 5, 5);
    frame54x_changed_ = true;
}

void VehicleClimate::receive(uint32_t id, uint8_t len, byte* data) {
    switch(id) {
        case 0x54A:
            receive54A(len, data);
            break;
        case 0x54B:
            receive54B(len, data);
            break;
        case 0x625:
            receive625(len, data);
            break;
    }
}

void VehicleClimate::receive54A(uint8_t len, byte* data) {
    if (len != 8) {
        ERROR_MSG_VAL("vehicle: frame 0x54A has invalid length: 8 != ", len);
        return;
    }
    INFO_MSG_FRAME("vehicle: receive ", 0x54A, 8, data);

    driver_temp_ = data[4];
    passenger_temp_ = data[5];

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
    mode_ = data[1];
    recirculate_ = getBit(data, 3, 4);
    fan_speed_ = (data[2] + 1) / 2;

    updateDash();
}

void VehicleClimate::receive625(uint8_t len, byte* data) {
    if (len != 6) {
        ERROR_MSG_VAL("vehicle: frame 0x625 has invalid length: 6 != ", len);
        return;
    }
    INFO_MSG_FRAME("vehicle: receive ", 0x625, 8, data);

    rear_defrost_ = getBit(data, 0, 0);
}

void VehicleClimate::updateDash() {
    if (dash_ == nullptr) {
        return;
    }

    dash_->setClimateActive(active_);
    dash_->setClimateRearDefrost(rear_defrost_);

    if (active_) {
        dash_->setClimateAuto(auto_);
        dash_->setClimateAc(ac_);
        dash_->setClimateDual(dual_);
        dash_->setClimateRecirculate(recirculate_);
        dash_->setClimateFanSpeed(fan_speed_);
        dash_->setClimateDriverTemp(driver_temp_);
        dash_->setClimatePassengerTemp(passenger_temp_);

        switch(mode_) {
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
            default:    // off
                dash_->setClimateFace(false);
                dash_->setClimateFeet(false);
                dash_->setClimateFrontDefrost(false);
                break;
        }
    } else {
        dash_->setClimateAuto(false);
        dash_->setClimateAc(false);
        dash_->setClimateDual(false);
        dash_->setClimateRecirculate(false);
        dash_->setClimateFanSpeed(0);
        dash_->setClimateDriverTemp(0);
        dash_->setClimatePassengerTemp(0);
        dash_->setClimateFace(false);
        dash_->setClimateFeet(false);
        dash_->setClimateFrontDefrost(false);
    }
}
