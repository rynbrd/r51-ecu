#include "Nav.h"

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>
#include <Faker.h>
#include "HMIEvent.h"

namespace R51 {
namespace {

using ::Caster::Yield;

static const uint32_t kPowerLongPressTimeout = 3000;

}

NavControls::NavControls(uint8_t encoder_keypad_id, Faker::Clock* clock) :
    keypad_id_(encoder_keypad_id),
    power_(kPowerLongPressTimeout, clock),
    page_(NavPage::NONE) {}

void NavControls::handle(const Message& msg, const Yield<Message>& yield) {
    if (msg.type() != Message::EVENT) {
        return;
    }
    switch ((SubSystem)msg.event().subsystem) {
        case SubSystem::HMI:
            if (msg.event().id == (uint8_t)HMIEvent::PAGE_STATE) {
                handlePage((DisplayPageState*)&msg.event());
            }
            break;
        case SubSystem::KEYPAD:
            if (msg.event().data[0] == keypad_id_) {
                switch (page_) {
                    case NavPage::AUDIO:
                        handleAudio(msg.event(), yield);
                        break;
                    case NavPage::CLIMATE:
                        handleClimate(msg.event(), yield);
                        break;
                    case NavPage::SETTINGS:
                        handleSettings(msg.event(), yield);
                        break;
                    default:
                        break;
                }
            }
            break;
        default:
            break;
    }
}

void NavControls::emit(const Yield<Message>& yield) {
    if (power_.trigger()) {
        switch (page_) {
            case NavPage::AUDIO:
                sendCmd(yield, AudioEvent::POWER_TOGGLE_CMD);
                break;
            case NavPage::CLIMATE:
                sendCmd(yield, ClimateEvent::TURN_OFF_CMD);
                break;
            default:
                break;
        }
    }
}

void NavControls::handlePage(const DisplayPageState* event) {
    switch (event->page()) {
        case HMIPage::SPLASH:
        case HMIPage::HOME:
        case HMIPage::VEHICLE:
        case HMIPage::SHARED:
        case HMIPage::AUDIO_NO_STEREO:
            setPage(NavPage::NONE);
            break;
        case HMIPage::CLIMATE:
            setPage(NavPage::CLIMATE);
            break;
        case HMIPage::AUDIO:
        case HMIPage::AUDIO_TRACK:
        case HMIPage::AUDIO_RADIO:
        case HMIPage::AUDIO_AUX:
        case HMIPage::AUDIO_POWER_OFF:
        case HMIPage::AUDIO_VOLUME:
            setPage(NavPage::AUDIO);
            break;
        case HMIPage::AUDIO_SOURCE:
        case HMIPage::AUDIO_SETTINGS:
        case HMIPage::AUDIO_EQ:
        case HMIPage::SETTINGS:
        case HMIPage::SETTINGS_1:
        case HMIPage::SETTINGS_2:
        case HMIPage::SETTINGS_3:
            setPage(NavPage::SETTINGS);
            break;
    }
}

void NavControls::handleAudio(const Event& event, const Yield<Message>& yield) {
    if (event.id == (uint8_t)KeypadEvent::KEY_STATE) {
        const auto* key = (KeyState*)&event;
        if (key->key() == 0 && !key->pressed()) {
            sendCmd(yield, AudioEvent::VOLUME_TOGGLE_MUTE_CMD);
        } else if (key->key() == 1) {
            if (key->pressed()) {
                power_.press();
            } else if (power_.release()) {
                sendCmd(yield, AudioEvent::PLAYBACK_TOGGLE_CMD);
            }
        }
    } else if (event.id == (uint8_t)KeypadEvent::ENCODER_STATE) {
        const auto* encoder = (EncoderState*)&event;
        if (encoder->encoder() == 0) {
            if (encoder->delta() > 0) {
                sendCmd(yield, AudioEvent::VOLUME_INC_CMD);
            } else {
                sendCmd(yield, AudioEvent::VOLUME_DEC_CMD);
            }
        } else if (encoder->encoder() == 1) {
            if (encoder->delta() > 0) {
                sendCmd(yield, AudioEvent::PLAYBACK_NEXT_CMD);
            } else {
                sendCmd(yield, AudioEvent::PLAYBACK_PREV_CMD);
            }
        }
    }
}

void NavControls::handleClimate(const Event& event, const Yield<Message>& yield) {
    if (event.id == (uint8_t)KeypadEvent::KEY_STATE) {
        const auto* key = (KeyState*)&event;
        if (key->key() == 0 && !key->pressed()) {
            sendCmd(yield, ClimateEvent::TOGGLE_AUTO_CMD);
        } else if (key->key() == 1 && !key->pressed()) {
            sendCmd(yield, ClimateEvent::TOGGLE_DUAL_CMD);
        }
    } else if (event.id == (uint8_t)KeypadEvent::ENCODER_STATE) {
        const auto* encoder = (EncoderState*)&event;
        if (encoder->encoder() == 0) {
            if (encoder->delta() > 0) {
                sendCmd(yield, ClimateEvent::INC_DRIVER_TEMP_CMD);
            } else {
                sendCmd(yield, ClimateEvent::DEC_DRIVER_TEMP_CMD);
            }
        } else if (encoder->encoder() == 1) {
            if (encoder->delta() > 0) {
                sendCmd(yield, ClimateEvent::INC_PASSENGER_TEMP_CMD);
            } else {
                sendCmd(yield, ClimateEvent::DEC_PASSENGER_TEMP_CMD);
            }
        }
    }
}

void NavControls::handleSettings(const Event& event, const Yield<Message>& yield) {
    if (event.id == (uint8_t)KeypadEvent::KEY_STATE) {
        const auto* key = (KeyState*)&event;
        if (key->key() == 0 && !key->pressed()) {
            sendCmd(yield, HMIEvent::NAV_ACTIVATE_CMD);
        }
    } else if (event.id == (uint8_t)KeypadEvent::ENCODER_STATE) {
        const auto* encoder = (EncoderState*)&event;
        if (encoder->encoder() == 0) {
            if (encoder->delta() > 0) {
                sendCmd(yield, HMIEvent::NAV_UP_CMD);
            } else {
                sendCmd(yield, HMIEvent::NAV_DOWN_CMD);
            }
        } else if (encoder->encoder() == 1) {
            if (encoder->delta() > 0) {
                sendCmd(yield, HMIEvent::NAV_RIGHT_CMD);
            } else {
                sendCmd(yield, HMIEvent::NAV_LEFT_CMD);
            }
        }
    }
}

void NavControls::setPage(NavPage page) {
    if (page == NavPage::NONE || page != page_) {
        power_.release();
    }
    page_ = page;
}

};
