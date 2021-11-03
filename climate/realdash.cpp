#include "realdash.h"

#include <Arduino.h>
#include "binary.h"
#include "debug.h"


RealDashReceiver::RealDashReceiver() {
    stream_ = nullptr;
    reset();
}

void RealDashReceiver::reset() {
    memset(checksum_buffer_, 0, 4);
    frame_type_66_ = false;
    frame44_checksum_ = 0;
    frame66_checksum_.reset();
    frame_size_ = 8;
    read_size_ = 0;
}

void RealDashReceiver::begin(Stream* stream) {
    stream_ = stream;
}

void RealDashReceiver::updateChecksum(byte b) {
    if (frame_type_66_) {
        frame66_checksum_.update(b);
    } else {
        frame44_checksum_ += b;
    }
}

bool RealDashReceiver::read(uint32_t* id, uint8_t* len, byte* data) {
    if (stream_ == nullptr) {
        ERROR_MSG("realdash: not initialized");
        return false;
    }
    if (readHeader() && readId(id) && readData(len, data) && validateChecksum()) {
        reset();
        return true;
    }
    return false;
}

bool RealDashReceiver::readHeader() {
    byte b;
    while (stream_->available() && read_size_ < 4) {
        b = stream_->read();
        read_size_++;
        switch (read_size_) {
            case 1:
                if (b != 0x44 && b != 0x66) {
                    ERROR_MSG_VAL_FMT("realdash: unrecognized frame type ", b, HEX);
                    reset();
                    return false;
                }
                frame_type_66_ = (b == 0x66);
                break;
            case 2:
                if (b != 0x33) {
                    ERROR_MSG_VAL_FMT("realdash: invalid header byte 2 ", b, HEX);
                    reset();
                    return false;
                }
                break;
            case 3:
                if (b != 0x22) {
                    ERROR_MSG_VAL_FMT("realdash: invalid header byte 3 ", b, HEX);
                    reset();
                    return false;
                }
                break;
            case 4:
                if ((!frame_type_66_ && b != 0x11) || (frame_type_66_ && (b < 0x11 || b > 0x1F))) {
                    ERROR_MSG_VAL_FMT("realdash: invalid header byte 4 ", b, HEX);
                    reset();
                    return false;
                }
                frame_size_ = (b - 15) * 4;
                break;
            default:
                break;
        }
        updateChecksum(b);
    }
    return read_size_ >= 4;
}

bool RealDashReceiver::readId(uint32_t* id) {
    uint32_t b;
    while (stream_->available() && read_size_ < 8) {
        b = stream_->read();
        read_size_++;
        updateChecksum(b);
        switch (read_size_-4) {
            case 1:
                *id = b;
                break;
            case 2:
                *id |= (b << 8);
                break;
            case 3:
                *id |= (b << 16);
                break;
            case 4:
                *id |= (b << 24);
                break;
        }
    }
    return read_size_ >= 8;
}

bool RealDashReceiver::readData(uint8_t* len, byte* data) {
    while (stream_->available() && read_size_ - 8 < frame_size_) {
        data[read_size_-8] = stream_->read();
        updateChecksum(data[read_size_-8]);
        read_size_++;
    }
    *len = frame_size_;
    return read_size_ - 8 >= frame_size_;
}

bool RealDashReceiver::validateChecksum() {
    if (frame_type_66_) {
        while (stream_->available() && read_size_ - 8 - frame_size_ < 4) {
            checksum_buffer_[read_size_ - 8 - frame_size_] = stream_->read();
            read_size_++;
        }
        if (read_size_ - 8 - frame_size_ < 4) {
            return false;
        }
        if (frame66_checksum_.finalize() != *((uint32_t*)checksum_buffer_)) {
            ERROR_MSG_VAL_FMT("realdash: frame 0x66 checksum error, wanted ", frame66_checksum_.finalize(), HEX);
            reset();
            return false;
        }
    } else {
        if (read_size_ - 8 - frame_size_ < 1) {
            if (!stream_->available()) {
                return false;
            }
            checksum_buffer_[0] = stream_->read();
            read_size_++;
        }
        if (frame44_checksum_ != checksum_buffer_[0]) {
            ERROR_MSG_VAL_FMT("realdash: frame 0x44 checksum error, wanted ", frame44_checksum_, HEX);
            reset();
            return false;
        }
    }
    return true;
}

void RealDashReceiver::writeByte(const byte b) {
    stream_->write(b);
    write_checksum_.update(b);
}

void RealDashReceiver::writeBytes(const byte* b, uint8_t len) {
    for (int i = 0; i < len; i++) {
        writeByte(b[i]);
    }
}

bool RealDashReceiver::write(uint32_t id, uint8_t len, byte* data) {
    if (stream_ == nullptr) {
        ERROR_MSG("realdash: not initialized");
        return false;
    }
    if (data == nullptr) {
        len = 0;
    }
    if (len > 64 || len % 4 != 0) {
        ERROR_MSG_VAL("realdash: frame write error, invalid length ", len);
        return false;
    }

    write_checksum_.reset();

    byte size = len / 4 + 15;
    writeByte(0x66);
    writeByte(0x33);
    writeByte(0x22);
    writeByte(size);
    writeBytes((const byte*)&id, 4);
    if (data != nullptr) {
        writeBytes(data, len);
    }
    for (int i = 0; i < 8-len; i++) {
        writeByte(0);
    }
    uint32_t checksum = write_checksum_.finalize();
    stream_->write((const byte*)&checksum, 4);
    stream_->flush();
    return true;
}

void RealDashController::connect(RealDashReceiver* realdash) {
    if (repeat_ < 1) {
        repeat_ = 1;
    }
    realdash_ = realdash;
    last_write_ = 0;
    write_count_ = repeat_;     // force a write on start
    memset(frame5400_, 0, 8);
}

void RealDashController::setClimateActive(bool value) {
    setBit(frame5400_, 0, 0, value);
    write_count_ = 0;
}

void RealDashController::setClimateAuto(bool value) {
    setBit(frame5400_, 0, 1, value);
    write_count_ = 0;
}

void RealDashController::setClimateAc(bool value) {
    setBit(frame5400_, 0, 2, value);
    write_count_ = 0;
}

void RealDashController::setClimateDual(bool value) {
    setBit(frame5400_, 0, 3, value);
    write_count_ = 0;
}

void RealDashController::setClimateFace(bool value) {
    setBit(frame5400_, 0, 4, value);
    write_count_ = 0;
}

void RealDashController::setClimateFeet(bool value) {
    setBit(frame5400_, 0, 5, value);
    write_count_ = 0;
}

void RealDashController::setClimateRecirculate(bool value) {
    setBit(frame5400_, 0, 7, value);
    write_count_ = 0;
}

void RealDashController::setClimateFrontDefrost(bool value) {
    setBit(frame5400_, 1, 6, value);
    write_count_ = 0;
}

void RealDashController::setClimateRearDefrost(bool value) {
    setBit(frame5400_, 4, 0, value);
    write_count_ = 0;
}

void RealDashController::setClimateMirrorDefrost(bool value) {
    setBit(frame5400_, 4, 1, value);
    write_count_ = 0;
}

void RealDashController::setClimateFanSpeed(uint8_t value) {
    frame5400_[1] = value;
    write_count_ = 0;
}

void RealDashController::setClimateDriverTemp(uint8_t value) {
    frame5400_[2] = value;
    write_count_ = 0;
}

void RealDashController::setClimatePassengerTemp(uint8_t value) {
    frame5400_[3] = value;
    write_count_ = 0;
}

void RealDashController::push() {
    if (realdash_ == nullptr) {
        return;
    }
    if (write_count_ < repeat_ || millis() - last_write_ >= 500) {
        if (write_count_ < repeat_) {
            INFO_MSG_FRAME("realdash: send ", 0x5400, 8, frame5400_);
            write_count_++;
        }
        realdash_->write(0x5400, 8, frame5400_);
    }
}

void RealDashListener::connect(ClimateController* climate) {
    memset(frame5401_, 0, 5);
    climate_ = climate;
}

void RealDashListener::receive(uint32_t id, uint8_t len, byte* data) {
    if (climate_ == nullptr || id != 0x5401 || len != 8) {
        return;
    }

    // check if any bits have flipped
    D(bool changed = false;)
    if (xorBits(frame5401_, data, 0, 0)) {
        climate_->deactivateClimate();
        D(changed = true;)
    }
    if (xorBits(frame5401_, data, 0, 1)) {
        climate_->toggleClimateAuto();
        D(changed = true;)
    }
    if (xorBits(frame5401_, data, 0, 2)) {
        climate_->toggleClimateAc();
        D(changed = true;)
    }
    if (xorBits(frame5401_, data, 0, 3)) {
        climate_->toggleClimateDual();
        D(changed = true;)
    }
    if (xorBits(frame5401_, data, 0, 4)) {
        climate_->cycleClimateMode();
        D(changed = true;)
    }
    if (xorBits(frame5401_, data, 0, 6)) {
        climate_->toggleClimateFrontDefrost();
        D(changed = true;)
    }
    if (xorBits(frame5401_, data, 0, 7)) {
        climate_->toggleClimateRecirculate();
        D(changed = true;)
    }
    if (xorBits(frame5401_, data, 1, 0)) {
        climate_->increaseClimateFanSpeed();
        D(changed = true;)
    }
    if (xorBits(frame5401_, data, 1, 1)) {
        climate_->decreaseClimateFanSpeed();
        D(changed = true;)
    }
    if (xorBits(frame5401_, data, 1, 2)) {
        climate_->increaseClimateDriverTemp(1);
        D(changed = true;)
    }
    if (xorBits(frame5401_, data, 1, 3)) {
        climate_->decreaseClimateDriverTemp(1);
        D(changed = true;)
    }
    if (xorBits(frame5401_, data, 1, 4)) {
        climate_->setClimateDriverTemp(data[2]);
        D(changed = true;)
    }
    if (xorBits(frame5401_, data, 1, 5)) {
        climate_->increaseClimatePassengerTemp(1);
        D(changed = true;)
    }
    if (xorBits(frame5401_, data, 1, 6)) {
        climate_->decreaseClimatePassengerTemp(1);
        D(changed = true;)
    }
    if (xorBits(frame5401_, data, 1, 7)) {
        climate_->setClimatePassengerTemp(data[3]);
        D(changed = true;)
    }
    if (xorBits(frame5401_, data, 4, 0)) {
        climate_->toggleClimateRearDefrost();
        D(changed = true;)
    }
    if (xorBits(frame5401_, data, 4, 1)) {
        climate_->toggleClimateMirrorDefrost();
        D(changed = true;)
    }

    D({
        if (changed) {
            INFO_MSG_FRAME("realdash: receive ", 0x5400, 8, frame5400_);
        }
    })

    // update the stored frame
    memcpy(frame5401_, data, 5);
}
