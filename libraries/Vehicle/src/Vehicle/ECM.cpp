#include "ECM.h"

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>

namespace R51 {

void EngineTempState::handle(const Message& msg) {
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

void EngineTempState::handleFrame(const Canny::Frame& frame) {
    if (frame.id() != 0x551 || frame.size() < 1) {
        return;
    }

    // The value sent by the ECM and our event are offset by -40 so we don't
    // need to perform any adjustements.
    uint8_t value = frame.data()[0];
    if (value != event_.data[0]) {
        event_.data[0] = value;
        changed_ = true;
    }
}

void EngineTempState::handleEvent(const Event& event) {
    if (event.subsystem != (uint8_t)SubSystem::ECM ||
            event.id != (uint8_t)ECMEvent::REQUEST) {
        return;
    }
    changed_ = true;
}

void EngineTempState::emit(const Caster::Yield<Message>& yield) {
    if (changed_ || ticker_.active()) {
        ticker_.reset();
        yield(event_);
        changed_ = false;
    }
}

}  // namespace R51
