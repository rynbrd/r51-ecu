#include "Nav.h"

#include <Arduino.h>
#include <Caster.h>
#include <Core.h>
#include <Faker.h>
#include "Screen.h"

namespace R51 {
namespace {

using ::Caster::Yield;

static const uint32_t kPowerLongPressTimeout = 3000;

static const uint8_t kBrightnessLow = 0x60;
static const uint8_t kBrightnessHigh = 0xFF;
static const uint8_t kBacklight = 0x10;
static const LEDColor kBacklightColor = LEDColor::AMBER;

}

NavControls::NavControls(uint8_t encoder_keypad_id, Faker::Clock* clock) :
    keypad_id_(encoder_keypad_id), power_(true), illum_(false),
    power_btn_(kPowerLongPressTimeout, clock),
    page_(NavPage::NONE) {}

void NavControls::handle(const Message& msg, const Yield<Message>& yield) {
    if (msg.type() != Message::EVENT) {
        return;
    }
    switch ((SubSystem)msg.event()->subsystem) {
        case SubSystem::SCREEN:
            switch ((ScreenEvent)msg.event()->id)  {
                case ScreenEvent::POWER_STATE:
                    handlePowerState((ScreenPowerState*)msg.event(), yield);
                    break;
                case ScreenEvent::PAGE_STATE:
                    handlePageState((ScreenPageState*)msg.event());
                    break;
                default:
                    break;
            }
            break;
        case SubSystem::KEYPAD:
            if (msg.event()->data[0] == keypad_id_) {
                if (!power_) {
                    handlePowerInput(*msg.event(), yield);
                } else {
                    switch (page_) {
                        case NavPage::AUDIO:
                            handleAudioInput(*msg.event(), yield);
                            break;
                        case NavPage::CLIMATE:
                            handleClimateInput(*msg.event(), yield);
                            break;
                        case NavPage::SETTINGS:
                            handleSettingsInput(*msg.event(), yield);
                            break;
                        default:
                            break;
                    }
                }
            }
            break;
        case SubSystem::BCM:
            if (msg.event()->id == (uint8_t)BCMEvent::ILLUM_STATE) {
                illum_ = msg.event()->data[0] != 0x00;
                illum(yield);
            }
        default:
            break;
    }
}

void NavControls::emit(const Yield<Message>& yield) {
    if (power_btn_.trigger()) {
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

void NavControls::handlePowerState(const ScreenPowerState* event, const Yield<Message>& yield) {
    power_ = event->power();
    illum(yield);
}

void NavControls::handlePageState(const ScreenPageState* event) {
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

void NavControls::handleAudioInput(const Event& event, const Yield<Message>& yield) {
    if (event.id == (uint8_t)KeypadEvent::KEY_STATE) {
        const auto* key = (KeyState*)&event;
        if (key->key() == 0) {
            if (key->pressed()) {
                power_btn_.press();
            } else if (power_btn_.release()) {
                sendCmd(yield, AudioEvent::VOLUME_TOGGLE_MUTE_CMD);
            }
        } else if (key->key() == 1 && !key->pressed()) {
            sendCmd(yield, AudioEvent::PLAYBACK_TOGGLE_CMD);
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

void NavControls::handleClimateInput(const Event& event, const Yield<Message>& yield) {
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

void NavControls::handleSettingsInput(const Event& event, const Yield<Message>& yield) {
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

void NavControls::handlePowerInput(const Event& event, const Caster::Yield<Message>& yield) {
    if (event.id == (uint8_t)KeypadEvent::KEY_STATE) {
        const auto* key = (KeyState*)&event;
        if (!key->pressed()) {
            sendCmd(yield, ScreenEvent::NAV_PAGE_NEXT_CMD);
        }
    } else if (event.id == (uint8_t)KeypadEvent::ENCODER_STATE) {
        const auto* encoder = (EncoderState*)&event;
        if (encoder->delta() != 0) {
            sendCmd(yield, ScreenEvent::NAV_PAGE_NEXT_CMD);
        }
    }
}

void NavControls::setPage(NavPage page) {
    if (page == NavPage::NONE || page != page_) {
        power_btn_.release();
    }
    page_ = page;
}

void NavControls::illum(const Caster::Yield<Message>& yield) {
    if (illum_) {
        // nighttime: backlight, low brightness indicators
        setBrightness(yield, keypad_id_, kBrightnessLow);
        setBacklight(yield, keypad_id_, power_ ? kBacklight : 0, kBacklightColor);
    } else {
        // daytime: no backlight, high brightness indicators
        setBrightness(yield, keypad_id_, kBrightnessHigh);
        setBacklight(yield, keypad_id_, 0);
    }
}

};
