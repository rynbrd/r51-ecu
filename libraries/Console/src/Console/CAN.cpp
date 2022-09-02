#include "CAN.h"

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include "Console.h"

namespace R51::internal {

void CANSendRunCommand::run(Console* console, const Caster::Yield<Message>& yield) {
    size_t len = strlen(buffer_);
    size_t pos = 0;
    size_t offset = 0;
    if (buffer_[0] == '+' || buffer_[0] == '-') {
        ++pos;
        offset = 1;
    }
    for (; pos < len; ++pos) {
        if (buffer_[pos] == '#') {
            buffer_[pos] = 0;
            ++pos;
            break;
        }
    }
    frame_.id(strtoul(buffer_ + offset, nullptr, 16));
    if (frame_.id() == 0) {
        console->stream()->println("console: invalid CAN frame format");
        return;
    }

    if (buffer_[0] == '+') {
        frame_.ext(1);
    } else if (buffer_[0] == '-') {
        frame_.ext(0);
    } else {
        frame_.ext(frame_.id() > 0x7FF ? 1 : 0);
    }

    byte tmp;
    offset = pos;
    int count = 0;
    int data_len = 0;
    for (; pos < len+1; ++pos) {
        if (count == 2 || buffer_[pos] == ':' || buffer_[pos] == 0) {
            tmp = buffer_[pos];
            buffer_[pos] = 0;
            buffer_[data_len++] = strtoul((char*)(buffer_ + offset), nullptr, 16);
            buffer_[pos] = tmp;
            if (buffer_[pos] == ':') {
                offset = pos + 1;
                count = 0;
            } else {
                offset = pos;
                count = 1;
            }
        } else {
            ++count;
        }
    }
    frame_.data((uint8_t*)buffer_, data_len);
    console->stream()->print("console: send frame ");
    console->stream()->println(frame_);
    yield(frame_);
}

}
