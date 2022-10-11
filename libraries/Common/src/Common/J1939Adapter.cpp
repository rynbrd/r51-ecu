#include "J1939Adapter.h"

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include "Event.h"
#include "Message.h"

namespace R51 {

using ::Canny::J1939Message;
using ::Caster::Yield;

J1939Adapter::J1939Adapter() : j1939_(0xFF00, Canny::NullAddress) {
    j1939_.reserve(8);
}

void J1939Adapter::handle(const Message& msg, const Yield<Message>& yield) {
    switch (msg.type()) {
        case Message::J1939_CLAIM:
            j1939_.source_address(msg.j1939_claim().address());
            break;
        case Message::J1939_MESSAGE:
            handleJ1939Message(msg.j1939_message(), yield);
            break;
        case Message::EVENT:
            handleEvent(msg.event(), yield);
            break;
        default:
            break;
    }
}

void J1939Adapter::handleEvent(const Event& event, const Yield<Message>& yield) {
    if (j1939_.source_address() == Canny::NullAddress || !writeFilter(event)) {
        return;
    }
    j1939_.resize(8);
    j1939_.data()[0] = event.subsystem;
    j1939_.data()[1] = event.id;
    memcpy(j1939_.data()+2, event.data, 6);
    yield(j1939_);
}

void J1939Adapter::handleJ1939Message(const J1939Message& msg, const Yield<Message>& yield) {
    if (msg.dest_address() != j1939_.source_address() ||
            msg.size() < 8 || msg.broadcast() || msg.pgn() != 0xEF00) {
        // ignore messages that don't have events in them
        return;
    }

    event_.subsystem = msg.data()[0];
    event_.id = msg.data()[1];
    memcpy(event_.data, msg.data()+2, 6);
    if (readFilter(event_)) {
        yield(event_);
    }
}

}  // namespace R51
