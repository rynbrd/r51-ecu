#include "realdash.h"

#include <Arduino.h>
#include "binary.h"
#include "debug.h"

static const byte frame44Prefix[4] = {0x44, 0x33, 0x22, 0x11};

void RealDashSerial::begin(Stream* stream) {
    stream_ = stream;
}

bool RealDashSerial::read(uint32_t* id, uint8_t* len, byte* data) {
    if (stream_ == nullptr) {
        return false;
    }

    // Read serial data into the incoming buffer.
    while (incoming_size_ < 17 && stream_->available() > 0) {
        incoming_buffer_[incoming_size_++] = stream_->read();
        if (incoming_buffer_[0] != frame44Prefix[0]) {
            incoming_size_ = 0;
        }
    }

    // Return if we have not received enough data.
    if (incoming_size_ < 17) {
        return false;
    }

    // Verify frame prefix.
    for (int i = 0; i < 4; i++ ) {
        if (incoming_buffer_[i] != frame44Prefix[i]) {
            return false;
        }
    }

    // Verify frame checksum.
    byte checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += incoming_buffer_[i];
    }
    if (checksum != incoming_buffer_[16]) {
        return false;
    }

    // Copy data to args.
    memcpy(id, incoming_buffer_ + 4, 4);
    memcpy(data, incoming_buffer_ + 8, 8);
    *len = 8;

    // Reset input buffer.
    incoming_size_ = 0;

    return true;
}

void RealDashSerial::write(uint32_t id, uint8_t len, byte* data) {
    if (stream_ == nullptr) {
        return;
    }
    if (len > 8) {
        len = 8;
    }
    stream_->write(frame44Prefix, 4);
    stream_->write((const byte*)&id, 4);
    stream_->write(data, len);
    for (int i = 0; i < 8-len; i++) {
        stream_->write((byte)0);
    }
}

RealDashController::RealDashController() {
    realdash_ = nullptr;
    memset(frame5400_, 0, 8);
    frame5400_changed_ = false;
}

void RealDashController::connect(RealDashSerial* realdash) {
    realdash_ = realdash;
}

void RealDashController::setClimateActive(bool value) {
    INFO_MSG_VAL("realdash: climate set active: ", value);
    setBit(frame5400_, 0, 0, value);
    frame5400_changed_ = true;
}

void RealDashController::setClimateAuto(bool value) {
    INFO_MSG_VAL("realdash: climate: set auto: ", value);
    setBit(frame5400_, 0, 1, value);
    frame5400_changed_ = true;
}

void RealDashController::setClimateAc(bool value) {
    INFO_MSG_VAL("realdash: climate: set a/c: ", value);
    setBit(frame5400_, 0, 2, value);
    frame5400_changed_ = true;
}

void RealDashController::setClimateDual(bool value) {
    INFO_MSG_VAL("realdash: climate: set dual zone: ", value);
    setBit(frame5400_, 0, 3, value);
    frame5400_changed_ = true;
}

void RealDashController::setClimateFace(bool value) {
    INFO_MSG_VAL("realdash: climate: set face vent: ", value);
    setBit(frame5400_, 0, 4, value);
    frame5400_changed_ = true;
}

void RealDashController::setClimateFeet(bool value) {
    INFO_MSG_VAL("realdash: climate: set feet vent: ", value);
    setBit(frame5400_, 0, 5, value);
    frame5400_changed_ = true;
}

void RealDashController::setClimateRecirculate(bool value) {
    INFO_MSG_VAL("realdash: climate: set recirculate: ", value);
    setBit(frame5400_, 0, 6, value);
    frame5400_changed_ = true;
}

void RealDashController::setClimateFrontDefrost(bool value) {
    INFO_MSG_VAL("realdash: climate: set front defrost: ", value);
    setBit(frame5400_, 0, 7, value);
    frame5400_changed_ = true;
}

void RealDashController::setClimateRearDefrost(bool value) {
    INFO_MSG_VAL("realdash: climate: set rear defrost: ", value);
    setBit(frame5400_, 1, 0, value);
    frame5400_changed_ = true;
}

void RealDashController::setClimateFanSpeed(uint8_t value) {
    INFO_MSG_VAL("realdash: climate: set fan speed: ", value);
    if (value <= 8) {
        frame5400_[2] = value;
        frame5400_changed_ = true;
    }
}

void RealDashController::setClimateDriverTemp(uint8_t value) {
    INFO_MSG_VAL("realdash: climate: set driver temperature: ", value);
    frame5400_[4] = value;
    frame5400_changed_ = true;
}

void RealDashController::setClimatePassengerTemp(uint8_t value) {
    INFO_MSG_VAL("realdash: climate: set passenger temperature: ", value);
    frame5400_[5] = value;
    frame5400_changed_ = true;
}

void RealDashController::push() {
    if (realdash_ == nullptr) {
        return;
    }
    if (frame5400_changed_) {
        INFO_MSG_FRAME("realdash: send ", 0x5400, 8, frame5400_);
        realdash_->write(0x5400, 8, frame5400_);
        frame5400_changed_ = false;
    }
}

void RealDashListener::connect(ClimateController* climate) {
    climate_ = climate;
}

void RealDashListener::receive(uint32_t id, uint8_t len, byte* data) {
    if (id != 0x5401) {
        return;
    }
    if (len != 8) {
        ERROR_MSG_VAL("realdash: frame 0x5401 has invalid length: 8 != ", len);
    }
    INFO_MSG_FRAME("realdash: receive ", id, len, data);

    if (getBit(data, 0, 0)) {
        climate_->deactivateClimate();
    }
    if (getBit(data, 0, 1)) {
        climate_->toggleClimateAuto();
    }
    if (getBit(data, 0, 2)) {
        climate_->toggleClimateAc();
    }
    if (getBit(data, 0, 3)) {
        climate_->toggleClimateDual();
    }
    if (getBit(data, 0, 4)) {
        climate_->toggleClimateRecirculate();
    }
    if (getBit(data, 0, 5)) {
        climate_->cycleClimateMode();
    }
    if (getBit(data, 0, 6)) {
        climate_->toggleClimateFrontDefrost();
    }
    if (getBit(data, 0, 7)) {
        climate_->toggleClimateRearDefrost();
    }
    if (getBit(data, 1, 0)) {
        climate_->increaseClimateFanSpeed();
    }
    if (getBit(data, 1, 1)) {
        climate_->decreaseClimateFanSpeed();
    }
    if (data[2] != 0) {
        climate_->setClimateDriverTemp(data[2]);
    }
    if (data[3] != 0) {
        climate_->setClimatePassengerTemp(data[3]);
    }
}
