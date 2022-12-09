#include "Event.h"

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>
#include "Console.h"

namespace R51::internal {

void EventSendRunCommand::run(Console* console, char* arg, const Caster::Yield<Message>& yield) {
    size_t len = strlen(arg);
    if (len < 5 || arg[2] != ':' || (
            arg[5] != '#' && arg[5] != 0)) {
        console->stream()->println("console: invalid event format");
        return;
    }

    arg[2] = 0;
    event_.subsystem = strtoul((char*)arg, nullptr, 16);
    arg[5] = 0;
    event_.id = strtoul((char*)arg+3, nullptr, 16);

    if (len <= 5) {
        memset(event_.data, 0xFF, 6);
        console->stream()->print("console: send event ");
        console->stream()->println(event_);
        yield(MessageView(&event_));
        return;
    }

    byte tmp;
    int begin = 6;
    int count = 0;
    int data_len = 0;
    for (size_t i = 6; i < len+1; ++i) {
        if (count == 2 || arg[i] == ':' || arg[i] == 0) {
            tmp = arg[i];
            arg[i] = 0;
            event_.data[data_len++] = strtoul((char*)(arg + begin), nullptr, 16);
            arg[i] = tmp;
            if (data_len == 6) {
                break;
            }
            if (arg[i] == ':') {
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
    for (int i = data_len; i < 6; ++i) {
        event_.data[i] = 0xFF;
    }
    console->stream()->print("console: send event ");
    console->stream()->println(event_);
    yield(MessageView(&event_));
}

}
