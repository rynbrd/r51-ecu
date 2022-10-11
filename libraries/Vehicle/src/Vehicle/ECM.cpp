#include "ECM.h"

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>

namespace R51 {

void EngineTempState::handle(const Message& msg, const Caster::Yield<Message>& yield) {
    switch (msg.type()) {
        case Message::CAN_FRAME:
            handleFrame(msg.can_frame(), yield);
            break;
        case Message::EVENT:
            handleEvent(msg.event(), yield);
            break;
        default:
            break;
    }
}

void EngineTempState::handleFrame(const Canny::Frame& frame, const Caster::Yield<Message>& yield) {
    if (frame.id() != 0x551 || frame.size() < 1) {
        return;
    }

    // The value sent by the ECM and our event are offset by -40 so we don't
    // need to perform any adjustements.
    uint8_t value = frame.data()[0];
    if (value != event_.data[0]) {
        event_.data[0] = value;
        yieldEvent(yield);
    }
}

void EngineTempState::handleEvent(const Event& event, const Caster::Yield<Message>& yield) {
    if (RequestCommand::match(event, SubSystem::ECM, (uint8_t)ECMEvent::ENGINE_TEMP_STATE)) {
        yieldEvent(yield);
    }
}

void EngineTempState::emit(const Caster::Yield<Message>& yield) {
    if (ticker_.active()) {
        yieldEvent(yield);
    }
}

void EngineTempState::yieldEvent(const Caster::Yield<Message>& yield) {
    ticker_.reset();
    yield(event_);
}

}  // namespace R51
