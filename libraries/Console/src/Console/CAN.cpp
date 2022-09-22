#include "CAN.h"

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include "Console.h"

namespace R51::internal {

void CANSendRunCommand::run(Console* console, char* arg, const Caster::Yield<Message>& yield) {
    size_t len = strlen(arg);
    size_t pos = 0;
    size_t offset = 0;
    if (arg[0] == '+' || arg[0] == '-') {
        ++pos;
        offset = 1;
    }
    for (; pos < len; ++pos) {
        if (arg[pos] == '#') {
            arg[pos] = 0;
            ++pos;
            break;
        }
    }
    frame_.id(strtoul(arg + offset, nullptr, 16));
    if (frame_.id() == 0) {
        console->stream()->println("console: invalid CAN frame format");
        return;
    }

    if (arg[0] == '+') {
        frame_.ext(1);
    } else if (arg[0] == '-') {
        frame_.ext(0);
    } else {
        frame_.ext(frame_.id() > 0x7FF ? 1 : 0);
    }

    byte tmp;
    offset = pos;
    int count = 0;
    int data_len = 0;
    for (; pos < len+1; ++pos) {
        if (count == 2 || arg[pos] == ':' || arg[pos] == 0) {
            tmp = arg[pos];
            arg[pos] = 0;
            arg[data_len++] = strtoul((char*)(arg + offset), nullptr, 16);
            arg[pos] = tmp;
            if (arg[pos] == ':') {
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
    frame_.data((uint8_t*)arg, data_len);
    console->stream()->print("console: send frame ");
    console->stream()->println(frame_);
    yield(frame_);
}

void CANFilterAllowRunCommand::run(Console* console, char* arg, const Caster::Yield<Message>&) {
    if (strcmp(arg, "all") == 0) {
        console->can_filter()->mode(Canny::FilterMode::ALLOW);
    } else {
        uint32_t id = strtoul(arg, nullptr, 16);
        if (id == 0) {
            console->stream()->println("invalid frame id");
        } else {
            console->can_filter()->allow(id);
        }
    }
}

void CANFilterDropRunCommand::run(Console* console, char* arg, const Caster::Yield<Message>&) {
    if (strcmp(arg, "all") == 0) {
        console->can_filter()->mode(Canny::FilterMode::DROP);
    } else {
        uint32_t id = strtoul(arg, nullptr, 16);
        if (id == 0) {
            console->stream()->println("invalid frame id");
        } else {
            console->can_filter()->drop(id);
        }
    }
}

}  // namespace R51::internal
