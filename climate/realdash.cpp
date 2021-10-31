#include "realdash.h"

#include <Arduino.h>
#include "binary.h"
#include "debug.h"

static const byte kFrame44Prefix[4] = {0x44, 0x33, 0x22, 0x11};

void RealDashReceiver::begin(Stream* stream) {
    stream_ = stream;
}

void frameReadError() {
    ERROR_MSG("realdash: frame read error, reseting");
}

bool RealDashReceiver::read(uint32_t* id, uint8_t* len, byte* data) {
    if (stream_ == nullptr) {
        ERROR_MSG("realdash: not initialized");
        return false;
    }

    // Read serial data into the incoming buffer.
    while (incoming_size_ < 17 && stream_->available() > 0) {
        incoming_buffer_[incoming_size_++] = stream_->read();
        if (incoming_size_ == 1 && incoming_buffer_[0] != kFrame44Prefix[0]) {
            frameReadError();
            incoming_size_ = 0;
        } else if (incoming_size_ == 2 && incoming_buffer_[1] != kFrame44Prefix[1]) {
            frameReadError();
            incoming_size_ = 0;
        } else if (incoming_size_ == 3 && incoming_buffer_[2] != kFrame44Prefix[2]) {
            frameReadError();
            incoming_size_ = 0;
        } else if (incoming_size_ == 4 && incoming_buffer_[3] != kFrame44Prefix[3]) {
            frameReadError();
            incoming_size_ = 0;
        }
    }

    // Return if we have not received enough data.
    if (incoming_size_ < 17) {
        return false;
    }

    // Verify frame checksum.
    byte checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += incoming_buffer_[i];
    }
    if (checksum != incoming_buffer_[16]) {
        ERROR_MSG_VAL_FMT("realdash: frame checksum does not match:", checksum, HEX);
        incoming_size_ = 0;
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

void RealDashReceiver::write(uint32_t id, uint8_t len, byte* data) {
    if (stream_ == nullptr) {
        return;
    }
    if (data == nullptr) {
        len = 0;
    } else if (len > 8) {
        len = 8;
    }
    stream_->write(kFrame44Prefix, 4);
    stream_->write((const byte*)&id, 4);
    if (data != nullptr) {
        stream_->write(data, len);
    }
    for (int i = 0; i < 8-len; i++) {
        stream_->write((byte)0);
    }
    stream_->flush();
}

RealDash::RealDash() {
    realdash_ = nullptr;
    climate_ = nullptr;
    memset(frame5400_, 0, 8);
    frame5400_changed_ = false;
    last_write_ = 0;
}

void RealDash::connect(RealDashReceiver* realdash, ClimateController* climate) {
    realdash_ = realdash;
    climate_ = climate;
}

void RealDash::setClimateActive(bool value) {
    INFO_MSG_VAL("realdash: climate set active: ", value);
    setBit(frame5400_, 0, 0, value);
    frame5400_changed_ = true;
}

void RealDash::setClimateAuto(bool value) {
    INFO_MSG_VAL("realdash: climate: set auto: ", value);
    setBit(frame5400_, 0, 1, value);
    frame5400_changed_ = true;
}

void RealDash::setClimateAc(bool value) {
    INFO_MSG_VAL("realdash: climate: set a/c: ", value);
    setBit(frame5400_, 0, 2, value);
    frame5400_changed_ = true;
}

void RealDash::setClimateDual(bool value) {
    INFO_MSG_VAL("realdash: climate: set dual zone: ", value);
    setBit(frame5400_, 0, 3, value);
    frame5400_changed_ = true;
}

void RealDash::setClimateFace(bool value) {
    INFO_MSG_VAL("realdash: climate: set face vent: ", value);
    setBit(frame5400_, 0, 5, value);
    frame5400_changed_ = true;
}

void RealDash::setClimateFeet(bool value) {
    INFO_MSG_VAL("realdash: climate: set feet vent: ", value);
    setBit(frame5400_, 0, 6, value);
    frame5400_changed_ = true;
}

void RealDash::setClimateRecirculate(bool value) {
    INFO_MSG_VAL("realdash: climate: set recirculate: ", value);
    setBit(frame5400_, 0, 7, value);
    frame5400_changed_ = true;
}

void RealDash::setClimateFrontDefrost(bool value) {
    INFO_MSG_VAL("realdash: climate: set front defrost: ", value);
    setBit(frame5400_, 1, 0, value);
    frame5400_changed_ = true;
}

void RealDash::setClimateRearDefrost(bool value) {
    INFO_MSG_VAL("realdash: climate: set rear defrost: ", value);
    setBit(frame5400_, 1, 1, value);
    frame5400_changed_ = true;
}

void RealDash::setClimateMirrorDefrost(bool value) {
    INFO_MSG_VAL("realdash: climate: set mirror defrost: ", value);
    setBit(frame5400_, 1, 2, value);
    frame5400_changed_ = true;
}

void RealDash::setClimateFanSpeed(uint8_t value) {
    INFO_MSG_VAL("realdash: climate: set fan speed: ", value);
    if (value <= 8) {
        frame5400_[2] = (frame5400_[2] & 0xF0) | value;
        frame5400_changed_ = true;
    }
}

void RealDash::setClimateDriverTemp(uint8_t value) {
    INFO_MSG_VAL("realdash: climate: set driver temperature: ", value);
    frame5400_[3] = value;
    frame5400_changed_ = true;
}

void RealDash::setClimatePassengerTemp(uint8_t value) {
    INFO_MSG_VAL("realdash: climate: set passenger temperature: ", value);
    frame5400_[4] = value;
    frame5400_changed_ = true;
}

void RealDash::receive(uint32_t id, uint8_t len, byte* data) {
    if (climate_ == nullptr || id != 0x5400) {
        return;
    }
    if (len != 8) {
        ERROR_MSG_VAL("realdash: frame 0x5400 has invalid length: 8 != ", len);
    }
    INFO_MSG_FRAME("realdash: receive ", id, len, data);

    bool bit = getBit(data, 0, 0);
    if (bit != getBit(frame5400_, 0, 0)) {
        if (bit) {
            climate_->deactivateClimate();
        } else {
            climate_->toggleClimateAuto();
        }
    }

    if (xorBits(frame5400_, data, 0, 1)) {
        climate_->toggleClimateAuto();
    }

    if (xorBits(frame5400_, data, 0, 2)) {
        climate_->toggleClimateAc();
    }

    if (xorBits(frame5400_, data, 0, 3)) {
        climate_->toggleClimateDual();
    }

    bit = getBit(data, 0, 4);
    if (setBitXor(data, 0, 4, bit)) {
        climate_->cycleClimateMode();
    }

    if (xorBits(frame5400_, data, 0, 7)) {
        climate_->toggleClimateRecirculate();
    }

    if (xorBits(frame5400_, data, 1, 0)) {
        climate_->toggleClimateFrontDefrost();
    }

    if (xorBits(frame5400_, data, 1, 1)) {
        climate_->toggleClimateRearDefrost();
    }

    if (xorBits(frame5400_, data, 1, 2)) {
        climate_->toggleClimateMirrorDefrost();
    }

    bit = getBit(data, 2, 4);
    if (setBitXor(data, 2, 4, bit)) {
        climate_->increaseClimateFanSpeed();
    }

    bit = getBit(data, 2, 5);
    if (setBitXor(data, 2, 5, bit)) {
        climate_->decreaseClimateFanSpeed();
    }

    if (data[3] != 0) {
        climate_->setClimateDriverTemp(data[2]);
    }
    if (data[4] != 0) {
        climate_->setClimatePassengerTemp(data[3]);
    }
}

void RealDash::push() {
    if (realdash_ == nullptr) {
        return;
    }
    if (frame5400_changed_ || millis() - last_write_ >= 500) {
        D(if (frame5400_changed_) {
          INFO_MSG_FRAME("realdash: send ", 0x5400, 8, frame5400_);
        })
        realdash_->write(0x5400, 8, frame5400_);
        frame5400_changed_ = false;
    }
}

