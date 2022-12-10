#include "IPDM.h"

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>
#include <Foundation.h>

namespace R51 {

void IPDM::handle(const Message& msg, const Caster::Yield<Message>& yield) {
    switch (msg.type()) {
        case Message::CAN_FRAME:
            handleFrame(*msg.can_frame(), yield);
            break;
        case Message::EVENT:
            handleEvent(*msg.event(), yield);
            break;
        default:
            break;
    }
}

void IPDM::handleFrame(const Canny::CAN20Frame& frame, const Caster::Yield<Message>& yield) {
    if (frame.id() != 0x625 || frame.size() < 6) {
        return;
    }

    uint8_t state = 0x00;
    // high beams
    setBit(&state, 0, 0, getBit(frame.data(), 1, 4));
    // low beams
    setBit(&state, 0, 1, getBit(frame.data(), 1, 5));
    // running lights
    setBit(&state, 0, 2, getBit(frame.data(), 1, 6));
    // fog lights
    setBit(&state, 0, 3, getBit(frame.data(), 1, 3));
    // defrost heaters
    setBit(&state, 0, 6, getBit(frame.data(), 0, 0));
    // a/c compressor
    setBit(&state, 0, 7, getBit(frame.data(), 1, 7));

    if (state != event_.data[0]) {
        event_.data[0] = state;
        yieldEvent(yield);
    }
}

void IPDM::handleEvent(const Event& event, const Caster::Yield<Message>& yield) {
    if (RequestCommand::match(event, SubSystem::IPDM,
            (uint8_t)IPDMEvent::POWER_STATE)) {
        yieldEvent(yield);
    }
}

void IPDM::emit(const Caster::Yield<Message>& yield) {
    if (ticker_.active()) {
        yieldEvent(yield);
    }
}

void IPDM::yieldEvent(const Caster::Yield<Message>& yield) {
    ticker_.reset();
    yield(MessageView(&event_));
}

}  // namespace R51
