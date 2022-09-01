#include "Event.h"

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>

namespace R51::internal {

void EventSendRunCommand::run(Stream* console, const Caster::Yield<Message>& yield) {
    size_t len = strlen(encoded_);
    if (len < 5 || encoded_[2] != ':' || (
            encoded_[5] != '#' && encoded_[5] != 0)) {
        console->println("console: invalid event format");
        return;
    }

    encoded_[2] = 0;
    event_.subsystem = strtoul((char*)encoded_, nullptr, 16);
    encoded_[5] = 0;
    event_.id = strtoul((char*)encoded_+3, nullptr, 16);

    if (len <= 5) {
        memset(event_.data, 0xFF, 6);
        console->print("console: send event ");
        console->println(event_);
        yield(event_);
        return;
    }

    byte tmp;
    int begin = 6;
    int count = 0;
    int data_len = 0;
    for (size_t i = 6; i < len+1; ++i) {
        if (count == 2 || encoded_[i] == ':' || encoded_[i] == 0) {
            tmp = encoded_[i];
            encoded_[i] = 0;
            event_.data[data_len++] = strtoul((char*)(encoded_ + begin), nullptr, 16);
            encoded_[i] = tmp;
            if (data_len == 6) {
                break;
            }
            if (encoded_[i] == ':') {
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
    console->print("console: send event ");
    console->println(event_);
    yield(event_);
}

}
