#include "Tires.h"

#include <Arduino.h>
#include <Canny.h>
#include <Common.h>
#include "Config.h"

namespace R51 {
namespace {

uint8_t getPressureValue(const Canny::Frame& frame, int tire) {
    if (getBit(frame.data(), 7, 7-tire)) {
        return frame.data()[2+tire];
    }
    return 0;
}

}  // namespace

TirePressureState::TirePressureState(ConfigStore* config, uint32_t tick_ms, Faker::Clock* clock) :
    config_(config), changed_(false),
    event_((uint8_t)SubSystem::TIRE, (uint8_t)TireEvent::PRESSURE_STATE, {0x00, 0x00, 0x00, 0x00}),
    ticker_(tick_ms, clock), map_{0, 1, 2, 3} {
        if (config_ != nullptr) {
            config_->loadTireMap(map_);
        }
    }

void TirePressureState::handle(const Message& msg) {
    switch (msg.type()) {
        case Message::CAN_FRAME:
            handleFrame(msg.can_frame());
            break;
        case Message::EVENT:
            handleEvent(msg.event());
            break;
        default:
            break;
    }
}

void TirePressureState::handleFrame(const Canny::Frame& frame) {
    if (frame.id() != 0x385 || frame.size() != 8) {
        return;
    }

    for (int i = 0; i < 4; i++) {
        uint8_t value = getPressureValue(frame, map_[i]);
        if (event_.data[i] != value) {
            event_.data[i] = value;
            changed_ = true;
        }
    }
}

void TirePressureState::handleEvent(const Event& event) {
    if (event.subsystem != (uint8_t)SubSystem::TIRE) {
        return;
    }

    switch ((TireEvent)event.id) {
        case TireEvent::REQUEST:
            changed_ = true;
            break;
        case TireEvent::SWAP_POSITION:
            swapPosition(event.data[0] & 0x0F, (event.data[0] & 0xF0) >> 4);
            if (config_ != nullptr) {
                config_->saveTireMap(map_);
            }
            break;
        default:
            break;
    }
}

void TirePressureState::emit(const Caster::Yield<Message>& yield) {
    if (changed_ || ticker_.active()) {
        ticker_.reset();
        yield(event_);
        changed_ = false;
    }
}

void TirePressureState::swapPosition(uint8_t a, uint8_t b) {
    if (a > 3 || b > 3 || a == b) {
        return;
    }

    // swap mapping
    uint8_t tmp = map_[a];
    map_[a] = map_[b];
    map_[b] = tmp;

    // swap stored values
    tmp = event_.data[a];
    event_.data[a] = event_.data[b];
    event_.data[b] = tmp;

    // trigger an emit
    changed_ = true;
}


}  // namespace R51
