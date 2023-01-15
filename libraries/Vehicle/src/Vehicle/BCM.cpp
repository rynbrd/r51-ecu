#include "BCM.h"

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include <Core.h>
#include <Foundation.h>
#include "Config.h"
#include "IPDM.h"

namespace R51 {
namespace {

uint8_t getPressureValue(const Canny::CAN20Frame& frame, int tire) {
    if (getBit(frame.data(), 7, 7-tire)) {
        return frame.data()[2+tire];
    }
    return 0;
}

}  // namespace

void Illum::handle(const Message& msg, const Caster::Yield<Message>& yield) {
    if (msg.type() != Message::EVENT) {
        return;
    }
    if ((RequestCommand::match(*msg.event(), SubSystem::BCM,
            (uint8_t)BCMEvent::ILLUM_STATE)) ||
            (msg.event()->subsystem == (uint8_t)SubSystem::IPDM &&
            msg.event()->id == (uint8_t)IPDMEvent::POWER_STATE &&
            state_.illum(getBit(msg.event()->data, 0, 0) || getBit(msg.event()->data, 0, 1)))) {
        yield(MessageView(&state_));
    }
}

void Defrost::begin() {
    output_.init();
}

void Defrost::handle(const Message& msg, const Caster::Yield<Message>&) {
    if (msg.type() != Message::EVENT ||
            msg.event()->subsystem != (uint8_t)SubSystem::BCM ||
            msg.event()->id != (uint8_t)BCMEvent::TOGGLE_DEFROST_CMD) {
        return;
    }
    output_.trigger();
}

void Defrost::emit(const Caster::Yield<Message>&) {
    output_.update();
}

TirePressure::TirePressure(ConfigStore* config, uint32_t tick_ms, Faker::Clock* clock) :
    config_(config), event_((uint8_t)SubSystem::BCM,
    (uint8_t)BCMEvent::TIRE_PRESSURE_STATE, (uint8_t[]){0x00, 0x00, 0x00, 0x00}),
    ticker_(tick_ms, tick_ms == 0, clock), map_{0, 1, 2, 3} {
        if (config_ != nullptr) {
            config_->loadTireMap(map_);
        }
    }

void TirePressure::handle(const Message& msg, const Caster::Yield<Message>& yield) {
    switch (msg.type()) {
        case Message::CAN_FRAME:
            handleFrame(*msg.can_frame(), yield);
            break;
        case Message::EVENT:
            handleEvent(*msg.event(), yield);
            break;
        default:
            break;
    }
}

void TirePressure::handleFrame(const Canny::CAN20Frame& frame,const Caster::Yield<Message>& yield) {
    if (frame.id() != 0x385 || frame.size() != 8) {
        return;
    }

    bool changed = false;
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t value = getPressureValue(frame, map_[i]);
        if (event_.data[i] != value) {
            event_.data[i] = value;
            changed = true;
        }
    }
    if (changed) {
        yieldEvent(yield);
    }
}

void TirePressure::handleEvent(const Event& event, const Caster::Yield<Message>& yield) {
    if (RequestCommand::match(event, SubSystem::BCM,
            (uint8_t)BCMEvent::TIRE_PRESSURE_STATE)) {
        yieldEvent(yield);
    }
    if (event.subsystem != (uint8_t)SubSystem::BCM ||
            event.id != (uint8_t)BCMEvent::TIRE_SWAP_CMD) {
        return;
    }

    swapPosition(event.data[0] & 0x0F, (event.data[0] & 0xF0) >> 4, yield);
    if (config_ != nullptr) {
        config_->saveTireMap(map_);
    }
}

void TirePressure::emit(const Caster::Yield<Message>& yield) {
    if (ticker_.active()) {
        yieldEvent(yield);
    }
}

void TirePressure::yieldEvent(const Caster::Yield<Message>& yield) {
    ticker_.reset();
    yield(MessageView(&event_));
}

void TirePressure::swapPosition(uint8_t a, uint8_t b, const Caster::Yield<Message>& yield) {
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

    // send an event
    yieldEvent(yield);
}

}  // namespace R51
