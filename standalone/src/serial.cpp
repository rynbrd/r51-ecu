#include "serial.h"

#include <Canny.h>
#include <Caster.h>

#include "config.h"
#include "debug.h"
#include "events.h"


void SerialText::begin(Stream* stream) {
    stream_ = stream;
    reset();
}

void SerialText::emit(const Caster::Yield<Message>& yield) {
    if (stream_ == nullptr) {
        return;
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
            return;
        }
        buffer_[buffer_len_++] = b;
    }

    if (!complete) {
        return;
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
        return;
    }

    data_len_ = buffer_len_ - id_len_ - 1;
    if (data_len_ % 2 == 1) {
        ERROR_MSG("serial: invalid frame format: odd number of bytes");
        reset();
        return;
    }

    memcpy(conv_, buffer_, id_len_);
    conv_[id_len_] = 0;
    frame_.id(strtoul((char*)conv_, nullptr, 16));
    if (frame_.id() == 0) {
        ERROR_MSG("serial: invalid frame format: bad id");
        reset();
        return;
    }

    frame_.resize(data_len_ / 2);
    conv_[2] = 0;
    for (int i = 0; i < frame_.size(); i++) {
        memcpy(conv_, buffer_ + (i * 2 + id_len_ + 1), 2);
        frame_.data()[i] = (byte)strtoul((char*)conv_, nullptr, 16);
    }

    reset();
    if (readFilter(frame_)) {
        yield(frame_);
    }
}

void SerialText::handle(const Message& msg) {
    if (msg.type() != Message::CAN_FRAME || !writeFilter(msg.can_frame())) {
        return;
    }
    const auto& frame = msg.can_frame();
    stream_->print(frame.id(), HEX);
    stream_->print("#");
    for (int i = 0; i < frame.size(); i++) {
        if (frame.data()[i] <= 0x0F) {
            stream_->print("0");
        }
        stream_->print(frame.data()[i], HEX);
        if (i < frame.size()-1) {
            stream_->print(":");
        }
    }
    stream_->println("");
}

void SerialText::reset() {
    memset(buffer_, 0, 28);
    buffer_len_ = 0;
    id_len_ = 0;
    data_len_ = 0;
}
