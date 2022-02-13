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
