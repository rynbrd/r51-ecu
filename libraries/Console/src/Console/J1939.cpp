#include "J1939.h"

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include "CAN.h"
#include "Command.h"
#include "Console.h"
#include "Error.h"

namespace R51::internal {

void J1939SendRunCommand::run(Console* console, char* arg, const Caster::Yield<Message>& yield) {
    size_t len = strlen(arg);
    size_t pos = 0;
    size_t offset = 0;
    for (; pos < len; ++pos) {
        if (arg[pos] == '#') {
            arg[pos] = 0;
            ++pos;
            break;
        }
    }
    msg_.id(strtoul(arg + offset, nullptr, 16));
    if (msg_.id() == 0) {
        console->stream()->println("console: invalid J1939 msessage format");
        return;
    }

    if (arg[0] == '+') {
        msg_.ext(1);
    } else if (arg[0] == '-') {
        msg_.ext(0);
    } else {
        msg_.ext(msg_.id() > 0x7FF ? 1 : 0);
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
    msg_.data((uint8_t*)arg, data_len);
    console->stream()->print("console: send message ");
    console->stream()->println(msg_);
    yield(msg_);
}

}  // namespace R51::internal
