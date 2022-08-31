#include "Console.h"

#include <Canny.h>
#include <Caster.h>
#include <Common.h>

namespace R51 {

bool Console::parseEvent() {
    if (buffer_len_ < 6 || buffer_[2] != ':' || (
            buffer_[5] != '#' && buffer_[5] != '\n' && buffer_[5] != '\r')) {
        print("invalid event format");
        return false;
    }

    buffer_[2] = 0;
    event_.subsystem = strtoul((char*)buffer_, nullptr, 16);
    buffer_[5] = 0;
    event_.id = strtoul((char*)buffer_+3, nullptr, 16);

    if (buffer_len_ <= 5) {
        memset(event_.data, 0xFF, 6);
        return true;
    }

    byte tmp;
    int begin = 6;
    int count = 0;
    int data_len = 0;
    for (int i = 6; i < buffer_len_; i++) {
        if (count == 2 || buffer_[i] == ':' || buffer_[i] == '\n' || buffer_[i] == '\r') {
            tmp = buffer_[i];
            buffer_[i] = 0;
            event_.data[data_len++] = strtoul((char*)(buffer_ + begin), nullptr, 16);
            buffer_[i] = tmp;
            if (data_len == 6) {
                break;
            }
            if (buffer_[i] == ':') {
                begin = i + 1;
                count = 0;
            } else {
                begin = i;
                count = 1;
            }
        } else {
            ++count;
        }
    }
    for (int i = data_len; i < 6; i++) {
        event_.data[i] = 0xFF;
    }
    return true;
}

void Console::emit(const Caster::Yield<Message>& yield) {
    if (stream_ == nullptr) {
        return;
    }

    byte b = 0;
    while (stream_->available()) {
        b = stream_->read();
        if (b == '\n' || b == '\r') {
            buffer_[buffer_len_++] = b;
            // break on newline
            if (parseEvent()) {
                yield(event_);
                print("send", event_);
            }
            reset();
        }
        if (buffer_len_ >= 24) {
            print("buffer overflow");
            reset();
            return;
        }
        buffer_[buffer_len_++] = b;
    }

}

void Console::handle(const Message& msg) {
    if (msg.type() == Message::EMPTY || !writeFilter(msg)) {
        return;
    }
    switch (msg.type()) {
        case Message::EVENT:
            print("recv ", msg.event());
            break;
        case Message::CAN_FRAME:
            print("recv ", msg.can_frame());
            break;
        case Message::EMPTY:
            break;
    }
}

void Console::reset() {
    buffer_len_ = 0;
}

void Console::print(const char* msg) {
    if (prefix_ != nullptr) {
        stream_->print(prefix_);
    }
    stream_->println(msg);
}

void Console::print(const char* msg, const Printable& p) {
    if (prefix_ != nullptr) {
        stream_->print(prefix_);
    }
    stream_->print(msg);
    stream_->println(p);
}

}  // namespace R51
