#include "Power.h"

#include <Caster.h>
#include <Common.h>
#include <Vehicle.h>
#include "Screen.h"

namespace R51 {
namespace {

using ::Caster::Yield;

static const uint8_t kBrightnessLow = 0x01;
static const uint8_t kBrightnessHigh = 0xC0;
static const uint8_t kBacklight = 0x01;
static const LEDColor kBacklightColor = LEDColor::AMBER;
static const LEDColor kLockerColor = LEDColor::RED;
static const LEDColor kAirColor = LEDColor::CYAN;
static const LEDColor kLightColor = LEDColor::YELLOW;

}  // namespace

PowerControls::PowerControls(uint8_t keypad_id, uint8_t pdm_id) :
    keypad_id_(keypad_id), pdm_id_(pdm_id),
    indicator_cmd_(keypad_id_) {}

void PowerControls::handle(const Message& msg, const Yield<Message>& yield) {
    if (msg.type() != Message::EVENT) {
        return;
    }
    if (msg.event().subsystem == (uint8_t)SubSystem::KEYPAD &&
            msg.event().id == (uint8_t)KeypadEvent::KEY_STATE &&
            msg.event().data[0] == keypad_id_) {
        handleKey((KeyState*)&msg.event(), yield);
    } else if (msg.event().subsystem == (uint8_t)SubSystem::POWER &&
            msg.event().id == (uint8_t)PowerEvent::POWER_STATE &&
            msg.event().data[0] == pdm_id_) {
        handlePower((PowerState*)&msg.event(), yield);
    } else if (msg.event().subsystem == (uint8_t)SubSystem::BCM &&
            msg.event().id == (uint8_t)BCMEvent::ILLUM_STATE) {
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
}

// blink keypad key map:
//   0: right arrow up
//   1: right arrow down
//   2: page/layer
//   3: left arrow up
//   4: left arrow down
//   5: power/circle
void PowerControls::handleKey(const KeyState* key, const Yield<Message>& yield) {
    if (key->pressed()) {
        return;
    }
    switch (key->key()) {
        case 0:
            sendPowerCmd(yield, PDMDevice::LIGHT_BAR, PowerCmd::TOGGLE);
            break;
        case 1:
            // noop
            break;
        case 2:
            sendCmd(yield, ScreenEvent::NAV_PAGE_NEXT_CMD);
            break;
        case 3:
            sendPowerCmd(yield, PDMDevice::FRONT_LOCKER, PowerCmd::TOGGLE);
            break;
        case 4:
            sendPowerCmd(yield, PDMDevice::REAR_LOCKER, PowerCmd::TOGGLE);
            break;
        case 5:
            sendPowerCmd(yield, PDMDevice::AIR_COMP, PowerCmd::TOGGLE);
            break;
        default:
            break;
    }
}

void PowerControls::handlePower(const PowerState* power, const Yield<Message>& yield) {
    switch ((PDMDevice)power->pin()) {
        case PDMDevice::FRONT_LOCKER:
            sendIndicatorCmd(yield, 3, power->mode(), power->duty_cycle(), kLockerColor);
            break;
        case PDMDevice::REAR_LOCKER:
            sendIndicatorCmd(yield, 4, power->mode(), power->duty_cycle(), kLockerColor);
            break;
        case PDMDevice::AIR_COMP:
            sendIndicatorCmd(yield, 5, power->mode(), power->duty_cycle(), kAirColor);
            break;
        case PDMDevice::LIGHT_BAR:
            sendIndicatorCmd(yield, 0, power->mode(), power->duty_cycle(), kLightColor);
            break;
    }
}

void PowerControls::sendIndicatorCmd(const Yield<Message>& yield, uint8_t led,
        PowerMode mode, uint8_t duty_cycle, LEDColor color) {
    indicator_cmd_.led(led);
    switch (mode) {
        case PowerMode::OFF:
            indicator_cmd_.mode(LEDMode::OFF);
            break;
        case PowerMode::ON:
            indicator_cmd_.mode(LEDMode::ON);
            indicator_cmd_.color(color);
            break;
        case PowerMode::PWM:
            if (duty_cycle > 0x00) {
                indicator_cmd_.mode(LEDMode::ON);
                indicator_cmd_.color(color);
            } else {
                indicator_cmd_.mode(LEDMode::OFF);
            }
            break;
        case PowerMode::FAULT:
            indicator_cmd_.mode(LEDMode::BLINK);
            indicator_cmd_.color(color);
            break;
    }
    yield(indicator_cmd_);
}

void PowerControls::sendPowerCmd(const Yield<Message>& yield, PDMDevice device, PowerCmd cmd) {
    Controls::sendPowerCmd(yield, pdm_id_, (uint8_t)device, cmd);
}

}  // namespace R51
