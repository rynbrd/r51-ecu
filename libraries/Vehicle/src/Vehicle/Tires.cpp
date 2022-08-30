#include "Tires.h"

#include <Arduino.h>
#include <Canny.h>
#include <Common.h>

namespace R51 {
namespace {

uint8_t getPressureValue(const Canny::Frame& frame, int tire) {
    if (getBit(frame.data(), 7, 7-tire)) {
        return frame.data()[2+tire];
    }
    return 0;
}

}  // namespace

TirePressureState::TirePressureState(uint32_t tick_ms, Faker::Clock* clock) :
    changed_(false),
    event_((uint8_t)SubSystem::TIRE, (uint8_t)TireEvent::PRESSURE_STATE, {0x00, 0x00, 0x00, 0x00}),
    ticker_(tick_ms, clock), map_{0, 1, 2, 3} {}

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
    //TODO: Implement state request.
    if (event.subsystem != (uint8_t)SubSystem::TIRE ||
            event.id != (uint8_t)TireEvent::SWAP_POSITION) {
        return;
    }

    uint8_t i = event.data[0] & 0x0F;
    uint8_t j = (event.data[0] & 0xF0) >> 4;
    if (i > 3 || j > 3 || i == j) {
        return;
    }

    // swap mapping
    uint8_t tmp = map_[i];
    map_[i] = map_[j];
    map_[j] = tmp;

    // swap stored values
    tmp = event_.data[i];
    event_.data[i] = event_.data[j];
    event_.data[j] = tmp;

    // trigger an emit
    changed_ = true;
}

void TirePressureState::emit(const Caster::Yield<Message>& yield) {
    if (changed_ || ticker_.active()) {
        ticker_.reset();
        yield(event_);
        changed_ = false;
    }
}

}  // namespace R51
