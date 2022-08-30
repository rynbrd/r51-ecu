#include "ECM.h"

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>

namespace R51 {

void EngineTempState::handle(const Message& msg) {
    if (msg.type() != Message::CAN_FRAME ||
            msg.can_frame().id() != 0x551 ||
            msg.can_frame().size() < 1) {
        return;
    }

    // The value sent by the ECM and our event are offset by -40 so we don't
    // need to perform any adjustements.
    uint8_t value = msg.can_frame().data()[0];
    if (value != event_.data[0]) {
        event_.data[0] = value;
        changed_ = true;
    }
}

void EngineTempState::emit(const Caster::Yield<Message>& yield) {
    if (changed_ || ticker_.active()) {
        ticker_.reset();
        yield(event_);
        changed_ = false;
    }
}

}  // namespace R51
