#include "vehicle.h"

#include <Arduino.h>
#include "binary.h"
#include "dash.h"
#include "debug.h"
#include "receiver.h"

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

void VehicleClimate::connect(Receiver* can, DashController* dash) {
    can_ = can;
    dash_ = dash;
}

bool VehicleClimate::climateOnline() const {
    return frame540_[0] == 0x60;
}

void VehicleClimate::deactivateClimate() {
    toggle(frame540_, 6, 7);
}

void VehicleClimate::toggleClimateAuto() {
    toggle(frame540_, 6, 5);
}

void VehicleClimate::toggleClimateAc() {
    toggle(frame540_, 5, 3);
}

void VehicleClimate::toggleClimateDual() {
    toggle(frame540_, 6, 3);
}

void VehicleClimate::toggleClimateRecirculate() {
    toggle(frame541_, 1, 6);
}


void VehicleClimate::cycleClimateMode() {
    toggle(frame540_, 6, 0);
}

void VehicleClimate::toggleClimateFrontDefrost() {
    toggle(frame540_, 6, 1);
}

void VehicleClimate::toggleClimateRearDefrost() {
    // TODO: determine rear defrost control signal
    if (climateOnline()) {
        INFO_MSG("vehicle: climate: toggle rear defrost (noop)");
    }
}

void VehicleClimate::toggleClimateMirrorDefrost() {
    // TODO: determine mirror defrost control signal
    if (climateOnline()) {
        INFO_MSG("vehicle: climate: toggle rear defrost (noop)");
    }
}

void VehicleClimate::increaseClimateFanSpeed() {
    toggle(frame541_, 0, 5);
}

void VehicleClimate::decreaseClimateFanSpeed() {
    toggle(frame541_, 0, 4);
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
    frame54x_changed_ = true;
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

    if (!dual_) {
        toggleClimateDual();
        dual_ = true;
    }

    toggleSetTemperatureBit();
    setPassengerTempByte(temp);
    frame54x_changed_ = true;
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

    if (dash_ != nullptr) {
        dash_->setClimateDriverTemp(driver_temp_);
        dash_->setClimatePassengerTemp(passenger_temp_);
    }
}

void VehicleClimate::receive54B(uint8_t len, byte* data) {
    if (len != 8) {
        ERROR_MSG_VAL("vehicle: frame 0x54B has invalid length: 8 != ", len);
        return;
    }
    INFO_MSG_FRAME("vehicle: receive ", 0x54B, 8, data);

    active_ = !getBit(data, 0, 5);
    dual_ = getBit(data, 3, 7);

    if (dash_ != nullptr) {
        dash_->setClimateActive(active_);
        dash_->setClimateAuto(getBit(data, 0, 0));
        dash_->setClimateAc(getBit(data, 0, 3));
        dash_->setClimateDual(dual_);
        dash_->setClimateRecirculate(getBit(data, 3, 4));
        dash_->setClimateFanSpeed((data[2] + 1) / 2);

        switch(data[1]) {
            case 0x04:
                dash_->setClimateFace(true);
                dash_->setClimateFeet(false);
                dash_->setClimateFrontDefrost(false);
                break;
            case 0x08:
                dash_->setClimateFace(true);
                dash_->setClimateFeet(true);
                dash_->setClimateFrontDefrost(false);
                break;
            case 0x0C:
                dash_->setClimateFace(false);
                dash_->setClimateFeet(true);
                dash_->setClimateFrontDefrost(false);
                break;
            case 0x10:
                dash_->setClimateFace(false);
                dash_->setClimateFeet(true);
                dash_->setClimateFrontDefrost(true);
                break;
            case 0x34:
                dash_->setClimateFace(false);
                dash_->setClimateFeet(false);
                dash_->setClimateFrontDefrost(true);
                break;
            case 0x84:
                dash_->setClimateFace(true);
                dash_->setClimateFeet(false);
                dash_->setClimateFrontDefrost(false);
                break;
            case 0x88:
                dash_->setClimateFace(true);
                dash_->setClimateFeet(true);
                dash_->setClimateFrontDefrost(false);
                break;
            case 0x8C:
                dash_->setClimateFace(false);
                dash_->setClimateFeet(true);
                dash_->setClimateFrontDefrost(false);
                break;
            default:
                dash_->setClimateFace(false);
                dash_->setClimateFeet(false);
                dash_->setClimateFrontDefrost(false);
                break;
        }
    }
}

void VehicleClimate::receive625(uint8_t len, byte* data) {
    if (len != 6) {
        ERROR_MSG_VAL("vehicle: frame 0x625 has invalid length: 6 != ", len);
        return;
    }
    INFO_MSG_FRAME("vehicle: receive ", 0x625, 8, data);
    dash_->setClimateRearDefrost(getBit(data, 0, 0));
}
