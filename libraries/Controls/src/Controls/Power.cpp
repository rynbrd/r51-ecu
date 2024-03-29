#include "Power.h"

#include <Caster.h>
#include <Core.h>
#include <Faker.h>
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
static const uint32_t kNavLongPressTimeout = 1000;

}  // namespace

PowerControls::PowerControls(uint8_t keypad_id, uint8_t pdm_id, Faker::Clock* clock) :
    keypad_id_(keypad_id), pdm_id_(pdm_id), power_(true), illum_(false),
    nav_button_(kNavLongPressTimeout, clock),
    indicator_cmd_(keypad_id_)  {}

void PowerControls::handle(const Message& msg, const Yield<Message>& yield) {
    if (msg.type() != Message::EVENT) {
        return;
    }
    if (msg.event()->subsystem == (uint8_t)SubSystem::KEYPAD &&
            msg.event()->id == (uint8_t)KeypadEvent::KEY_STATE &&
            msg.event()->data[0] == keypad_id_) {
        handleKey((KeyState*)msg.event(), yield);
    } else if (msg.event()->subsystem == (uint8_t)SubSystem::IPDM &&
            msg.event()->id == (uint8_t)IPDMEvent::POWER_STATE) {
        handleIPDM(msg.event(), yield);
    } else if (msg.event()->subsystem == (uint8_t)SubSystem::POWER &&
            msg.event()->id == (uint8_t)PowerEvent::POWER_STATE &&
            msg.event()->data[0] == pdm_id_) {
        handlePower((PowerState*)msg.event(), yield);
    } else if (msg.event()->subsystem == (uint8_t)SubSystem::SCREEN &&
            msg.event()->id == (uint8_t)ScreenEvent::POWER_STATE) {
        handlePowerState((ScreenPowerState*)msg.event(), yield);
    } else if (msg.event()->subsystem == (uint8_t)SubSystem::BCM &&
            msg.event()->id == (uint8_t)BCMEvent::ILLUM_STATE) {
        illum_ = msg.event()->data[0] != 0x00;
        illum(yield);
    }
}

void PowerControls::emit(const Yield<Message>& yield) {
    if (nav_button_.trigger()) {
        sendCmd(yield, ScreenEvent::NAV_HOME_CMD);
    }
}

void PowerControls::handleKey(const KeyState* key, const Yield<Message>& yield) {
    if (key->pressed()) {
        if (key->key() == 7) {
            nav_button_.press();
        }
        return;
    }
    switch (key->key()) {
        case 0:
            sendPowerCmd(yield, PDMDevice::LIGHT_BAR, PowerCmd::TOGGLE);
            break;
        case 1:
            sendPowerCmd(yield, PDMDevice::LEFT_SPOT_LIGHT, PowerCmd::TOGGLE);
            sendPowerCmd(yield, PDMDevice::RIGHT_SPOT_LIGHT, PowerCmd::TOGGLE);
            break;
        case 2:
            sendPowerCmd(yield, PDMDevice::CHASE_LIGHTS, PowerCmd::TOGGLE);
            break;
        case 3:
            sendPowerCmd(yield, PDMDevice::ROCK_LIGHTS, PowerCmd::TOGGLE);
            break;
        case 4:
            sendPowerCmd(yield, PDMDevice::FRONT_LOCKER, PowerCmd::TOGGLE);
            break;
        case 5:
            sendPowerCmd(yield, PDMDevice::REAR_LOCKER, PowerCmd::TOGGLE);
            break;
        case 6:
            sendPowerCmd(yield, PDMDevice::AIR_COMP, PowerCmd::TOGGLE);
            break;
        case 7:
            if (nav_button_.release()) {
                sendCmd(yield, ScreenEvent::NAV_PAGE_NEXT_CMD);
            }
            break;
        default:
            break;
    }
}

void PowerControls::handleIPDM(const Event* event, const Yield<Message>& yield) {
    if (event->id != (uint8_t)IPDMEvent::POWER_STATE) {
        return;
    }
    bool power = getBit(event->data, 0,  2);
    sendPowerCmd(yield, PDMDevice::RUNNING_LIGHTS, power ? PowerCmd::ON : PowerCmd::OFF);
}

void PowerControls::handlePower(const PowerState* power, const Yield<Message>& yield) {
    switch ((PDMDevice)power->pin()) {
        case PDMDevice::LIGHT_BAR:
            sendIndicatorCmd(yield, 0, power->mode(), power->duty_cycle(), kLightColor);
            break;
        case PDMDevice::LEFT_SPOT_LIGHT:
            sendIndicatorCmd(yield, 1, power->mode(), power->duty_cycle(), kLightColor);
            break;
        case PDMDevice::CHASE_LIGHTS:
            sendIndicatorCmd(yield, 2, power->mode(), power->duty_cycle(), kLightColor);
            break;
        case PDMDevice::ROCK_LIGHTS:
            sendIndicatorCmd(yield, 3, power->mode(), power->duty_cycle(), kLightColor);
            break;
        case PDMDevice::FRONT_LOCKER:
            sendIndicatorCmd(yield, 4, power->mode(), power->duty_cycle(), kLockerColor);
            break;
        case PDMDevice::REAR_LOCKER:
            sendIndicatorCmd(yield, 5, power->mode(), power->duty_cycle(), kLockerColor);
            break;
        case PDMDevice::AIR_COMP:
            sendIndicatorCmd(yield, 6, power->mode(), power->duty_cycle(), kAirColor);
            break;
    }
}

void PowerControls::handlePowerState(const ScreenPowerState* event, const Yield<Message>& yield) {
    power_ = event->power();
    illum(yield);
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
    yield(MessageView(&indicator_cmd_));
}

void PowerControls::sendPowerCmd(const Yield<Message>& yield, PDMDevice device, PowerCmd cmd) {
    Controls::sendPowerCmd(yield, pdm_id_, (uint8_t)device, cmd);
}

void PowerControls::illum(const Caster::Yield<Message>& yield) {
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

}  // namespace R51
