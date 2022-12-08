#include "Controls.h"

#include <Arduino.h>
#include <Bluetooth.h>
#include <Caster.h>
#include <Common.h>
#include <Vehicle.h>
#include "Audio.h"
#include "Screen.h"

namespace R51 {

using ::Caster::Yield;

void Controls::sendCmd(const Yield<Message>& yield, AudioEvent cmd) {
    event_.subsystem = (uint8_t)SubSystem::AUDIO;
    event_.id = (uint8_t)cmd;
    event_.data[0] = 0xFF;
    event_.data[1] = 0xFF;
    event_.data[2] = 0xFF;
    yield(MessageView(&event_));
}

void Controls::sendCmd(const Yield<Message>& yield, AudioEvent cmd, uint8_t payload) {
    event_.subsystem = (uint8_t)SubSystem::AUDIO;
    event_.id = (uint8_t)cmd;
    event_.data[0] = payload;
    event_.data[1] = 0xFF;
    event_.data[2] = 0xFF;
    yield(MessageView(&event_));
}

void Controls::sendCmd(const Yield<Message>& yield, AudioEvent cmd, AudioSource payload) {
    event_.subsystem = (uint8_t)SubSystem::AUDIO;
    event_.id = (uint8_t)cmd;
    event_.data[0] = (uint8_t)payload;
    event_.data[1] = 0xFF;
    event_.data[2] = 0xFF;
    yield(MessageView(&event_));
}

void Controls::sendCmd(const Yield<Message>& yield, ClimateEvent cmd) {
    event_.subsystem = (uint8_t)SubSystem::CLIMATE;
    event_.id = (uint8_t)cmd;
    event_.data[0] = 0xFF;
    event_.data[1] = 0xFF;
    event_.data[2] = 0xFF;
    yield(MessageView(&event_));
}

void Controls::sendCmd(const Yield<Message>& yield, SettingsEvent cmd) {
    event_.subsystem = (uint8_t)SubSystem::SETTINGS;
    event_.id = (uint8_t)cmd;
    event_.data[0] = 0xFF;
    event_.data[1] = 0xFF;
    event_.data[2] = 0xFF;
    yield(MessageView(&event_));
}

void Controls::sendCmd(const Yield<Message>& yield, BCMEvent cmd) {
    event_.subsystem = (uint8_t)SubSystem::BCM;
    event_.id = (uint8_t)cmd;
    event_.data[0] = 0xFF;
    event_.data[1] = 0xFF;
    event_.data[2] = 0xFF;
    yield(MessageView(&event_));
}

void Controls::sendCmd(const Yield<Message>& yield, BCMEvent cmd, uint8_t payload) {
    event_.subsystem = (uint8_t)SubSystem::BCM;
    event_.id = (uint8_t)cmd;
    event_.data[0] = payload;
    event_.data[1] = 0xFF;
    event_.data[2] = 0xFF;
    yield(MessageView(&event_));
}

void Controls::sendCmd(const Yield<Message>& yield, BluetoothEvent cmd) {
    event_.subsystem = (uint8_t)SubSystem::BLUETOOTH;
    event_.id = (uint8_t)cmd;
    event_.data[0] = 0xFF;
    event_.data[1] = 0xFF;
    event_.data[2] = 0xFF;
    yield(MessageView(&event_));
}

void Controls::sendCmd(const Yield<Message>& yield, ScreenEvent cmd) {
    event_.subsystem = (uint8_t)SubSystem::SCREEN;
    event_.id = (uint8_t)cmd;
    event_.data[0] = 0xFF;
    event_.data[1] = 0xFF;
    event_.data[2] = 0xFF;
    yield(MessageView(&event_));
}

void Controls::sendPowerCmd(const Yield<Message>& yield, uint8_t pdm, uint8_t pin, PowerCmd cmd) {
    event_.subsystem = (uint8_t)SubSystem::POWER;
    event_.id = (uint8_t)PowerEvent::POWER_CMD;
    event_.data[0] = pdm;
    event_.data[1] = pin;
    event_.data[2] = (uint8_t)cmd;
    yield(MessageView(&event_));
}

void Controls::request(const Caster::Yield<Message>& yield, SubSystem subsystem, uint8_t id) {
    event_.subsystem = (uint8_t)SubSystem::CONTROLLER;
    event_.id = (uint8_t)ControllerEvent::REQUEST_CMD;
    event_.data[0] = (uint8_t)subsystem;
    event_.data[1] = (uint8_t)id;
    event_.data[2] = 0xFF;
    yield(MessageView(&event_));
}

void Controls::setBrightness(const Yield<Message>& yield, uint8_t keypad, uint8_t value) {
    event_.subsystem = (uint8_t)SubSystem::KEYPAD;
    event_.id = (uint8_t)KeypadEvent::BRIGHTNESS_CMD;
    event_.data[0] = keypad;
    event_.data[1] = value;
    event_.data[2] = 0xFF;
    yield(MessageView(&event_));
}

void Controls::setBacklight(const Yield<Message>& yield, uint8_t keypad,
        uint8_t brightness, LEDColor color) {
    event_.subsystem = (uint8_t)SubSystem::KEYPAD;
    event_.id = (uint8_t)KeypadEvent::BACKLIGHT_CMD;
    event_.data[0] = keypad;
    event_.data[1] = brightness;
    event_.data[2] = (uint8_t)color;
    yield(MessageView(&event_));
}

void RepeatButton::press() {
    ticker_.resume();
}

bool RepeatButton::trigger() {
    if (ticker_.active()) {
        ticker_.reset();
        return true;
    }
    return false;
}

bool RepeatButton::release() {
    ticker_.pause();
    return !ticker_.triggered();
}

void LongPressButton::press() {
    pressed_ = clock_->millis();
    state_ = 1;
}

bool LongPressButton::trigger() {
    if (state_ == 1 && clock_->millis() - pressed_ >= timeout_) {
        state_ = 2;
        return true;
    }
    return false;
}

bool LongPressButton::release() {
    bool ret = state_ == 1;
    state_ = 0;
    return ret;
}

}  // namespace R51
