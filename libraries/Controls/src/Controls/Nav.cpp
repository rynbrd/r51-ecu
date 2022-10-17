#include "Nav.h"

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>
#include <Faker.h>
#include "Screen.h"

namespace R51 {
namespace {

using ::Caster::Yield;

static const uint32_t kPowerLongPressTimeout = 3000;

static const uint8_t kBrightnessLow = 0x10;
static const uint8_t kBrightnessHigh = 0x40;
static const uint8_t kBacklight = 0x10;
static const LEDColor kBacklightColor = LEDColor::AMBER;

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
        case SubSystem::SCREEN:
            if (msg.event().id == (uint8_t)ScreenEvent::PAGE_STATE) {
                handlePage((ScreenPageState*)&msg.event());
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
        case SubSystem::BCM:
            if (msg.event().id == (uint8_t)BCMEvent::ILLUM_STATE) {
                if (msg.event().data[0] == 0x00) {
                    // daytime: no backlight, high brightness indicators
                    setBrightness(yield, keypad_id_, kBrightnessHigh);
                    setBacklight(yield, keypad_id_, 0);
                } else {
                    // nighttime: backlight, low brightness indicators
                    setBrightness(yield, keypad_id_, kBrightnessLow);
                    setBacklight(yield, keypad_id_, kBacklight, kBacklightColor);
                }
            }
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

void NavControls::handlePage(const ScreenPageState* event) {
    switch (event->page()) {
        case ScreenPage::CLIMATE:
            setPage(NavPage::CLIMATE);
            break;
        case ScreenPage::AUDIO:
        case ScreenPage::AUDIO_TRACK:
        case ScreenPage::AUDIO_RADIO:
        case ScreenPage::AUDIO_AUX:
        case ScreenPage::AUDIO_POWER_OFF:
        case ScreenPage::AUDIO_VOLUME:
            setPage(NavPage::AUDIO);
            break;
        case ScreenPage::AUDIO_SOURCE:
        case ScreenPage::AUDIO_SETTINGS:
        case ScreenPage::AUDIO_EQ:
        case ScreenPage::SETTINGS:
        case ScreenPage::SETTINGS_1:
        case ScreenPage::SETTINGS_2:
        case ScreenPage::SETTINGS_3:
            setPage(NavPage::SETTINGS);
            break;
        default:
            setPage(NavPage::NONE);
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
            sendCmd(yield, ScreenEvent::NAV_ACTIVATE_CMD);
        }
    } else if (event.id == (uint8_t)KeypadEvent::ENCODER_STATE) {
        const auto* encoder = (EncoderState*)&event;
        if (encoder->encoder() == 0) {
            if (encoder->delta() > 0) {
                sendCmd(yield, ScreenEvent::NAV_UP_CMD);
            } else {
                sendCmd(yield, ScreenEvent::NAV_DOWN_CMD);
            }
        } else if (encoder->encoder() == 1) {
            if (encoder->delta() > 0) {
                sendCmd(yield, ScreenEvent::NAV_RIGHT_CMD);
            } else {
                sendCmd(yield, ScreenEvent::NAV_LEFT_CMD);
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
