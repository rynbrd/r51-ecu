#include "Controls.h"

#include <Arduino.h>
#include <Bluetooth.h>
#include <Caster.h>
#include <Common.h>
#include <Vehicle.h>
#include "Audio.h"

namespace R51 {

using ::Caster::Yield;

void Controls::sendCmd(const Yield<Message>& yield, AudioEvent cmd) {
    event_.subsystem = (uint8_t)SubSystem::AUDIO;
    event_.id = (uint8_t)cmd;
    event_.data[0] = 0xFF;
    yield(event_);
}

void Controls::sendCmd(const Yield<Message>& yield, AudioEvent cmd, uint8_t payload) {
    event_.subsystem = (uint8_t)SubSystem::AUDIO;
    event_.id = (uint8_t)cmd;
    event_.data[0] = payload;
    yield(event_);
}

void Controls::sendCmd(const Yield<Message>& yield, AudioEvent cmd, AudioSource payload) {
    event_.subsystem = (uint8_t)SubSystem::AUDIO;
    event_.id = (uint8_t)cmd;
    event_.data[0] = (uint8_t)payload;
    yield(event_);
}

void Controls::sendCmd(const Yield<Message>& yield, ClimateEvent cmd) {
    event_.subsystem = (uint8_t)SubSystem::CLIMATE;
    event_.id = (uint8_t)cmd;
    event_.data[0] = 0xFF;
    yield(event_);
}

void Controls::sendCmd(const Yield<Message>& yield, SettingsEvent cmd) {
    event_.subsystem = (uint8_t)SubSystem::SETTINGS;
    event_.id = (uint8_t)cmd;
    event_.data[0] = 0xFF;
    yield(event_);
}

void Controls::sendCmd(const Yield<Message>& yield, IPDMEvent cmd) {
    event_.subsystem = (uint8_t)SubSystem::IPDM;
    event_.id = (uint8_t)cmd;
    event_.data[0] = 0xFF;
    yield(event_);
}

void Controls::sendCmd(const Yield<Message>& yield, TireEvent cmd, uint8_t payload) {
    event_.subsystem = (uint8_t)SubSystem::TIRE;
    event_.id = (uint8_t)cmd;
    event_.data[0] = payload;
    yield(event_);
}

void Controls::sendCmd(const Yield<Message>& yield, BluetoothEvent cmd) {
    event_.subsystem = (uint8_t)SubSystem::BLUETOOTH;
    event_.id = (uint8_t)cmd;
    event_.data[0] = 0xFF;
    yield(event_);
}

void Controls::sendCmd(const Yield<Message>& yield, HMIEvent cmd) {
    event_.subsystem = (uint8_t)SubSystem::HMI;
    event_.id = (uint8_t)cmd;
    event_.data[0] = 0xFF;
    yield(event_);
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
