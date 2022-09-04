#include "IPDM.h"

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>

namespace R51 {

void IPDM::handle(const Message& msg) {
    switch (msg.type()) {
        case Message::CAN_FRAME:
            handleFrame(msg.can_frame());
            break;
        case Message::EVENT:
            handleEvent(msg.event());
            break;
        default:
            break;
    }
}

void IPDM::handleFrame(const Canny::Frame& frame) {
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
    // defog heaters
    setBit(&state, 0, 6, getBit(frame.data(), 0, 0));
    // a/c compressor
    setBit(&state, 0, 7, getBit(frame.data(), 1, 7));

    if (state != event_.data[0]) {
        event_.data[0] = state;
        changed_ = true;
    }
}

void IPDM::handleEvent(const Event& event) {
    if (event.subsystem != (uint8_t)SubSystem::IPDM ||
            event.id != (uint8_t)IPDMEvent::REQUEST) {
        return;
    }
    changed_ = true;
}

void IPDM::emit(const Caster::Yield<Message>& yield) {
    if (changed_ || ticker_.active()) {
        ticker_.reset();
        yield(event_);
        changed_ = false;
    }
}

void Defog::handle(const Message& message) {
    if (message.type() != Message::EVENT ||
            message.event().subsystem !=  (uint8_t)SubSystem::IPDM ||
            message.event().id != (uint8_t)IPDMEvent::TOGGLE_DEFOG) {
        return;
    }
    output_.trigger();
}

void Defog::emit(const Caster::Yield<Message>&) {
    output_.update();
}

}  // namespace R51
