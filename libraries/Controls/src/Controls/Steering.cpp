#include "Steering.h"

#include <Caster.h>
#include <Core.h>
#include <Vehicle.h>
#include "Audio.h"

namespace R51 {
namespace {

using ::Caster::Yield;

static const uint32_t kPowerLongPressTimeout = 3000;
static const uint32_t kKeyRepeatInterval = 500;

}  // namespace

SteeringControls::SteeringControls(uint8_t steering_keypad_id, Faker::Clock* clock) :
    keypad_id_(steering_keypad_id),
    power_(kPowerLongPressTimeout, clock),
    seek_up_(kKeyRepeatInterval, clock),
    seek_down_(kKeyRepeatInterval, clock),
    volume_up_(kKeyRepeatInterval, clock),
    volume_down_(kKeyRepeatInterval, clock) {}

void SteeringControls::handle(const Message& msg, const Yield<Message>& yield) {
    if (msg.type() != Message::EVENT ||
            msg.event()->subsystem != (uint8_t)SubSystem::KEYPAD) {
        return;
    }

    const auto* key = (KeyState*)msg.event();
    if (key->keypad() != keypad_id_) {
        return;
    }

    switch ((SteeringKey)key->key()) {
        case SteeringKey::POWER:
            if (key->pressed()) {
                power_.press();
            } else if (power_.release()) {
                sendCmd(yield, AudioEvent::PLAYBACK_TOGGLE_CMD);
            }
            break;
        case SteeringKey::MODE:
            if (!key->pressed()) {
                sendCmd(yield, AudioEvent::SOURCE_NEXT_CMD);
            }
            break;
        case SteeringKey::SEEK_UP:
            if (key->pressed()) {
                seek_up_.press();
            } else if (seek_up_.release()) {
                sendCmd(yield, AudioEvent::PLAYBACK_NEXT_CMD);
            }
            break;
        case SteeringKey::SEEK_DOWN:
            if (key->pressed()) {
                seek_down_.press();
            } else if (seek_down_.release()) {
                sendCmd(yield, AudioEvent::PLAYBACK_PREV_CMD);
            }
            break;
        case SteeringKey::VOLUME_UP:
            if (key->pressed()) {
                volume_up_.press();
            } else if (volume_up_.release()) {
                sendCmd(yield, AudioEvent::VOLUME_INC_CMD);
            }
            break;
        case SteeringKey::VOLUME_DOWN:
            if (key->pressed()) {
                volume_down_.press();
            } else if (volume_down_.release()) {
                sendCmd(yield, AudioEvent::VOLUME_DEC_CMD);
            }
            break;
    }
}

void SteeringControls::emit(const Yield<Message>& yield) {
    if (power_.trigger()) {
        sendCmd(yield, AudioEvent::POWER_TOGGLE_CMD);
    }
    if (seek_up_.trigger()) {
        sendCmd(yield, AudioEvent::PLAYBACK_PREV_CMD);
    }
    if (seek_down_.trigger()) {
        sendCmd(yield, AudioEvent::PLAYBACK_NEXT_CMD);
    }
    if (volume_up_.trigger()) {
        sendCmd(yield, AudioEvent::VOLUME_INC_CMD);
    }
    if (volume_down_.trigger()) {
        sendCmd(yield, AudioEvent::VOLUME_DEC_CMD);
    }
}

}  // namespace R51
