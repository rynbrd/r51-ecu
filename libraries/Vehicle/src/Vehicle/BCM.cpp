#include "BCM.h"

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>

namespace R51 {

void Defrost::handle(const Message& message, const Caster::Yield<Message>&) {
    if (message.type() != Message::EVENT ||
            message.event().subsystem !=  (uint8_t)SubSystem::BCM ||
            message.event().id != (uint8_t)BCMEvent::TOGGLE_DEFROST_CMD) {
        return;
    }
    output_.trigger();
}

void Defrost::emit(const Caster::Yield<Message>&) {
    output_.update();
}

}  // namespace R51
