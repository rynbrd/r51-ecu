#include "serial.h"

#include "debug.h"

void SerialReceiver::begin(Stream* stream) {
    stream_ = stream;
    reset();
}

bool SerialReceiver::read(uint32_t* id, uint8_t* len, byte* data) {
    if (stream_ == nullptr) {
        return false;
    }

    byte b = 0;
    bool complete = false;
    while (stream_->available()) {
        b = stream_->read();
        if (b == ':') {
            // ignore byte delimeters
            continue;
        }
        if (b == '\n' || b == '\r') {
            // break on newline
            complete = true;
            break;
        }
        if (buffer_len_ >= 32) {
            ERROR_MSG("serial: buffer overflow");
            reset();
            return false;
        }
        buffer_[buffer_len_++] = b;
    }

    if (!complete) {
        return false;
    }

    for (int i = 0; i < buffer_len_; i++) {
        if (buffer_[i] == '#') {
            id_len_ = i;
            break;
        }
    }
    if (id_len_ > 8) {
        ERROR_MSG("serial: invalid frame format: id too long");
        reset();
        return false;
    }

    data_len_ = buffer_len_ - id_len_ - 1;
    if (data_len_ % 2 == 1) {
        ERROR_MSG("serial: invalid frame format: odd number of bytes");
        reset();
        return false;
    }

    memcpy(conv_, buffer_, id_len_);
    conv_[id_len_] = 0;
    *id = strtoul((char*)conv_, nullptr, 16);
    if (*id == 0) {
        ERROR_MSG("serial: invalid frame format: bad id");
        reset();
        return false;
    }

    data_len_ = 
    *len = data_len_ / 2;
    conv_[2] = 0;
    for (int i = 0; i < *len; i++) {
        memcpy(conv_, buffer_ + (i * 2 + id_len_ + 1), 2);
        data[i] = (byte)strtoul((char*)conv_, nullptr, 16);
    }

    reset();
    return true;
}

bool SerialReceiver::write(uint32_t id, uint8_t len, byte* data) {
    stream_->print(id, HEX);
    stream_->print("#");
    for (int i = 0; i < len; i++) {
        if (data[i] <= 0x0F) {
            stream_->print("0");
        }
        stream_->print(data[i], HEX);
        if (i < len-1) {
            stream_->print(":");
        }
    }
    stream_->print("\n");
    return true;
}

void SerialReceiver::reset() {
    memset(buffer_, 0, 28);
    buffer_len_ = 0;
    id_len_ = 0;
    data_len_ = 0;
}
