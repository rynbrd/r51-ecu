#include "realdash.h"

#include <Arduino.h>
#include "binary.h"
#include "config.h"
#include "debug.h"


static const uint32_t kReceiveTimeout = 5000;

RealDash::RealDash() {
    stream_ = nullptr;
    reset();
}

void RealDash::reset() {
    memset(checksum_buffer_, 0, 4);
    frame_type_66_ = false;
    frame44_checksum_ = 0;
    frame66_checksum_.reset();
    frame_size_ = 8;
    read_size_ = 0;
}

void RealDash::begin(Stream* stream) {
    stream_ = stream;
}

void RealDash::updateChecksum(byte b) {
    if (frame_type_66_) {
        frame66_checksum_.update(b);
    } else {
        frame44_checksum_ += b;
    }
}

void RealDash::receive(const Broadcast& broadcast) {
    if (stream_ == nullptr) {
        ERROR_MSG("realdash: not initialized");
        return;
    }
    if (!stream_) {
        ERROR_MSG("realdash: not connected");
        return;
    }
    if (readHeader() && readId() && readData() && validateChecksum()) {
        reset();
        broadcast(frame_);
    }
}

bool RealDash::readHeader() {
    byte b;
    while (stream_->available() && read_size_ < 4) {
        b = stream_->read();
        read_size_++;
        switch (read_size_) {
            case 1:
                if (b != 0x44 && b != 0x66) {
                    ERROR_MSG_VAL_FMT("realdash: unrecognized frame type ", b, HEX);
                    reset();
                    continue;
                }
                frame_type_66_ = (b == 0x66);
                break;
            case 2:
                if (b != 0x33) {
                    ERROR_MSG_VAL_FMT("realdash: invalid header byte 2 ", b, HEX);
                    reset();
                    continue;
                }
                break;
            case 3:
                if (b != 0x22) {
                    ERROR_MSG_VAL_FMT("realdash: invalid header byte 3 ", b, HEX);
                    reset();
                    continue;
                }
                break;
            case 4:
                if ((!frame_type_66_ && b != 0x11) || (frame_type_66_ && (b < 0x11 || b > 0x1F))) {
                    ERROR_MSG_VAL_FMT("realdash: invalid header byte 4 ", b, HEX);
                    reset();
                    continue;
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

bool RealDash::readId() {
    uint32_t b;
    while (stream_->available() && read_size_ < 8) {
        b = stream_->read();
        read_size_++;
        updateChecksum(b);
        switch (read_size_-4) {
            case 1:
                frame_.id = b;
                break;
            case 2:
                frame_.id |= (b << 8);
                break;
            case 3:
                frame_.id |= (b << 16);
                break;
            case 4:
                frame_.id |= (b << 24);
                break;
        }
    }
    return read_size_ >= 8;
}

bool RealDash::readData() {
    while (stream_->available() && read_size_ - 8 < frame_size_) {
        frame_.data[read_size_-8] = stream_->read();
        updateChecksum(frame_.data[read_size_-8]);
        read_size_++;
    }
    frame_.len = frame_size_;
    return read_size_ - 8 >= frame_size_;
}

bool RealDash::validateChecksum() {
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

void RealDash::writeByte(const byte b) {
    stream_->write(b);
    write_checksum_.update(b);
}

void RealDash::writeBytes(const byte* b, uint8_t len) {
    for (int i = 0; i < len; i++) {
        writeByte(b[i]);
    }
}

void RealDash::send(const Frame& frame) {
    if (stream_ == nullptr) {
        ERROR_MSG("realdash: not initialized");
        return;
    }
    if (!stream_) {
        ERROR_MSG("realdash: not connected");
        return;
    }
    if (frame.len > 64 || frame.len % 4 != 0) {
        ERROR_MSG_VAL("realdash: frame write error, invalid length ", frame.len);
        return;
    }

    write_checksum_.reset();

    byte size = (frame.len < 8 ? 8 : frame.len) / 4 + 15;
    writeByte(0x66);
    writeByte(0x33);
    writeByte(0x22);
    writeByte(size);
    writeBytes((const byte*)&(frame.id), 4);
    if (frame.data != nullptr) {
        writeBytes(frame.data, frame.len);
    }
    for (int i = 0; i < 8-frame.len; i++) {
        writeByte(0);
    }
    uint32_t checksum = write_checksum_.finalize();
    stream_->write((const byte*)&checksum, 4);
    stream_->flush();
}

void RealDashSettings::connect(Connection* realdash, SettingsController* settings) {
    if (repeat_ < 1) {
        repeat_ = 1;
    }
    realdash_ = realdash;
    settings_ = settings;
    last_read_ = millis();      // for control state timeouts
    write_count_ = repeat_;     // force a write on start
    memset(frame5700_, 0, 8);
    memset(frame5701_, 0, 8);
}

void RealDashSettings::setAutoInteriorIllumination(bool value) {
    setBit(frame5700_, 0, 0, value);
    write_count_ = 0;
}

void RealDashSettings::setAutoHeadlightSensitivity(uint8_t value) {
    if (value > 3) {
        value = 3;
    }
    frame5700_[1] &= ~0x03;
    frame5700_[1] |= value;
    write_count_ = 0;
}

void RealDashSettings::setAutoHeadlightOffDelay(AutoHeadlightOffDelay value) {
    frame5700_[1] &= ~0xF0;
    frame5700_[1] |= value << 4;
    write_count_ = 0;
}

void RealDashSettings::setSpeedSensingWiperInterval(bool value) {
    setBit(frame5700_, 0, 2, value);
    write_count_ = 0;
}

void RealDashSettings::setRemoteKeyResponseHorn(bool value) {
    setBit(frame5700_, 3, 0, value);
    write_count_ = 0;
}

void RealDashSettings::setRemoteKeyResponseLights(RemoteKeyResponseLights value) {
    frame5700_[3] &= ~0x30;
    frame5700_[3] |= value << 2;
    write_count_ = 0;
}

void RealDashSettings::setAutoReLockTime(AutoReLockTime value) {
    frame5700_[2] &= ~0xF0;
    frame5700_[2] |= value << 4;
    write_count_ = 0;
}

void RealDashSettings::setSelectiveDoorUnlock(bool value) {
    setBit(frame5700_, 2, 0, value);
    write_count_ = 0;
}

void RealDashSettings::setSlideDriverSeatBackOnExit(bool value) {
    setBit(frame5700_, 0, 1, value);
    write_count_ = 0;
}

void RealDashSettings::push() {
    if (realdash_ == nullptr) {
        return;
    }
    if (write_count_ < repeat_) {
        if (write_count_ == 0) {
            INFO_MSG_FRAME("realdash: send ", 0x5700, 8, frame5700_);
        }
        if (write_count_ < repeat_) {
            write_count_++;
        }
        realdash_->write(0x5700, 8, frame5700_);
    }
}

void RealDashSettings::receive(uint32_t id, uint8_t len, byte* data) {
    if (settings_ == nullptr || id != 0x5701 || len != 8) {
        return;
    }

    D(bool changed = false;)

    // reset internal state if we haven't received a frame recently
    if (millis() - last_read_ > kReceiveTimeout) {
        INFO_MSG("realdash: settings state reset, control frame timeout exceeded");
        memset(frame5701_, 0, 8);
        D(changed = true;)
    }
    last_read_ = millis();

    // check if any bits have flipped
    if (xorBits(frame5701_, data, 0, 0)) {
        settings_->toggleAutoInteriorIllumination();
        D(changed = true;)
    }
    if (xorBits(frame5701_, data, 0, 1)) {
        settings_->toggleSlideDriverSeatBackOnExit();
        D(changed = true;)
    }
    if (xorBits(frame5701_, data, 0, 2)) {
        settings_->toggleSpeedSensingWiperInterval();
        D(changed = true;)
    }
    if (xorBits(frame5701_, data, 1, 0)) {
        settings_->nextAutoHeadlightSensitivity();
        D(changed = true;)
    }
    if (xorBits(frame5701_, data, 1, 1)) {
        settings_->prevAutoHeadlightSensitivity();
        D(changed = true;)
    }
    if (xorBits(frame5701_, data, 1, 4)) {
        settings_->nextAutoHeadlightOffDelay();
        D(changed = true;)
    }
    if (xorBits(frame5701_, data, 1, 5)) {
        settings_->prevAutoHeadlightOffDelay();
        D(changed = true;)
    }
    if (xorBits(frame5701_, data, 2, 0)) {
        settings_->toggleSelectiveDoorUnlock();
        D(changed = true;)
    }
    if (xorBits(frame5701_, data, 2, 4)) {
        settings_->nextAutoReLockTime();
        D(changed = true;)
    }
    if (xorBits(frame5701_, data, 2, 5)) {
        settings_->prevAutoReLockTime();
        D(changed = true;)
    }
    if (xorBits(frame5701_, data, 3, 0)) {
        settings_->toggleRemoteKeyResponseHorn();
        D(changed = true;)
    }
    if (xorBits(frame5701_, data, 3, 2)) {
        settings_->nextRemoteKeyResponseLights();
        D(changed = true;)
    }
    if (xorBits(frame5701_, data, 3, 3)) {
        settings_->prevRemoteKeyResponseLights();
        D(changed = true;)
    }
    if (xorBits(frame5701_, data, 7, 0)) {
        settings_->retrieveSettings();
        D(changed = true;)
    }
    if (xorBits(frame5701_, data, 7, 7)) {
        settings_->resetSettingsToDefault();
        D(changed = true;)
    }

    // update the stored frame
    memcpy(frame5701_, data, 8);

    D({
        if (changed) {
            INFO_MSG_FRAME("realdash: receive ", 0x5701, 8, frame5701_);
        }
    })
}
