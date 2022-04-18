#include "climate.h"

#include <Canny.h>
#include <Caster.h>
#include <Faker.h>
#include <NissanR51.h>

#include "binary.h"
#include "config.h"
#include "debug.h"
#include "events.h"


Climate::Climate(Faker::Clock* clock, Faker::GPIO* gpio) : clock_(clock),
        rear_defrost_(REAR_DEFROST_PIN, REAR_DEFROST_TRIGGER_MS, clock, gpio),
        state_frame_(CLIMATE_STATE_FRAME_ID, 0, 8) {
    // Init state storage.
    state_init_ = 0;
    state_changed_ = false;
    state_last_broadcast_ = 0;

    // Init control storage.
    control_init_ = false;
    control_last_broadcast_ = 0;
    memset(control_state_, 0, 8);
}

void Climate::emit(const Caster::Yield<Message>& yield) {
    uint32_t control_hb = control_init_ ? CLIMATE_CONTROL_FRAME_HB : CLIMATE_CONTROL_INIT_HB;
    rear_defrost_.update();

    if (!control_init_ && clock_->millis() >= CLIMATE_CONTROL_INIT_EXPIRE) {
        system_control_.ready();
        fan_control_.ready();
        control_init_ = true;
    }

    if (system_control_.available() || fan_control_.available() ||
            clock_->millis() - control_last_broadcast_ >= control_hb) {
        control_last_broadcast_ = clock_->millis();
        yield(system_control_.frame());
        yield(fan_control_.frame());
    }

    if (state_init_ == 0x03 && (state_changed_ ||
            clock_->millis() - state_last_broadcast_ >= CLIMATE_STATE_FRAME_HB)) {
        state_changed_ = false;
        state_last_broadcast_ = clock_->millis();
        yield(state_frame_);
    }
}

void Climate::handle(const Message& msg) {
    if (msg.type() != Message::CAN_FRAME) {
        return;
    }
    handleTemps(msg.can_frame());
    handleSystem(msg.can_frame());
    handle625(msg.can_frame());
    handleControl(msg.can_frame());
}

void Climate::handleTemps(const Canny::Frame& frame) {
    if (frame.id() != 0x54A || frame.size() < 8) {
        return;
    }
    state_init_ |= 0x01;
    if (!temp_state_.handle(frame)) {
        return;
    }
    setDriverTemp(temp_state_.driver_temp());
    setPassengerTemp(temp_state_.passenger_temp());
    setOutsideTemp(temp_state_.outside_temp());
}

void Climate::handleSystem(const Canny::Frame& frame) {
    if (frame.id() != 0x54B || frame.size() < 8) {
        return;
    }
    state_init_ |= 0x02;
    if (!system_state_.handle(frame)) {
        return;
    }

    switch (system_state_.system()) {
        case NissanR51::CLIMATE_SYSTEM_OFF: 
            setActive(false);
            setAuto(false);
            setAc(false);
            setDual(false);
            setRecirculate(false);
            setFrontDefrost(false);
            setFanSpeed(0);
            setDriverTemp(0);
            setPassengerTemp(0);
            setMode(NissanR51::CLIMATE_VENTS_CLOSED);
            break;
        case NissanR51::CLIMATE_SYSTEM_AUTO:
            setActive(true);
            setAuto(true);
            setAc(system_state_.ac());
            setDual(system_state_.dual());
            setRecirculate(system_state_.recirculate());
            setFrontDefrost(false);
            setFanSpeed(system_state_.fan_speed());
            setMode(system_state_.vents());
            break;
        case NissanR51::CLIMATE_SYSTEM_MANUAL:
            setActive(true);
            setAuto(false);
            setAc(system_state_.ac());
            setDual(system_state_.dual());
            setRecirculate(system_state_.recirculate());
            setFrontDefrost(false);
            setFanSpeed(system_state_.fan_speed());
            setMode(system_state_.vents());
            break;
        case NissanR51::CLIMATE_SYSTEM_DEFROST:
            setActive(true);
            setAuto(false);
            setAc(system_state_.ac());
            setDual(false);
            setRecirculate(false);
            setFrontDefrost(true);
            setFanSpeed(system_state_.fan_speed());
            setMode(NissanR51::CLIMATE_VENTS_WINDSHIELD);
            break;
    }
}

void Climate::handle625(const Canny::Frame& frame) {
    if (frame.id() != 0x625 || frame.size() < 6) {
        return;
    }
    setRearDefrost(getBit(frame.data(), 0, 0));
}

void Climate::handleControl(const Canny::Frame& frame) {
    if (frame.id() != CLIMATE_CONTROL_FRAME_ID || frame.size() < 8) {
        return;
    }
    // check if any bits have flipped
    if (xorBits(control_state_, frame.data(), 0, 0)) {
        system_control_.turnOff();
    }
    if (xorBits(control_state_, frame.data(), 0, 1)) {
        system_control_.toggleAuto();
    }
    if (xorBits(control_state_, frame.data(), 0, 2)) {
        system_control_.toggleAC();
    }
    if (xorBits(control_state_, frame.data(), 0, 3)) {
        system_control_.toggleDual();
    }
    if (xorBits(control_state_, frame.data(), 0, 4)) {
        system_control_.cycleMode();
    }
    if (xorBits(control_state_, frame.data(), 0, 6)) {
        system_control_.toggleDefrost();
    }
    if (xorBits(control_state_, frame.data(), 0, 7)) {
        fan_control_.toggleRecirculate();
    }
    if (xorBits(control_state_, frame.data(), 1, 0)) {
        fan_control_.incFanSpeed();
    }
    if (xorBits(control_state_, frame.data(), 1, 1)) {
        fan_control_.decFanSpeed();
    }
    if (system_state_.system() != NissanR51::CLIMATE_SYSTEM_OFF) {
        if (xorBits(control_state_, frame.data(), 1, 2)) {
            system_control_.incDriverTemp();
        }
        if (xorBits(control_state_, frame.data(), 1, 3)) {
            system_control_.decDriverTemp();
        }
        if (xorBits(control_state_, frame.data(), 1, 4)) {
            system_control_.incPassengerTemp();
        }
        if (xorBits(control_state_, frame.data(), 1, 5)) {
            system_control_.decPassengerTemp();
        }
    }
    if (xorBits(control_state_, frame.data(), 4, 0)) {
        rear_defrost_.trigger();
    }

    // update the stored control state
    memcpy(control_state_, frame.data(), 8);
}

void Climate::setActive(bool value) {
    state_changed_ |= setBit(state_frame_.data(), 0, 0, value);
}

void Climate::setAuto(bool value) {
    state_changed_ |= setBit(state_frame_.data(), 0, 1, value);
}

void Climate::setAc(bool value) {
    state_changed_ |= setBit(state_frame_.data(), 0, 2, value);
}

void Climate::setDual(bool value) {
    state_changed_ |= setBit(state_frame_.data(), 0, 3, value);
}

void Climate::setFace(bool value) {
    state_changed_ |= setBit(state_frame_.data(), 0, 4, value);
}

void Climate::setFeet(bool value) {
    state_changed_ |= setBit(state_frame_.data(), 0, 5, value);
}

void Climate::setRecirculate(bool value) {
    state_changed_ |= setBit(state_frame_.data(), 0, 7, value);
}

void Climate::setFrontDefrost(bool value) {
    state_changed_ |= setBit(state_frame_.data(), 0, 6, value);
}

void Climate::setRearDefrost(bool value) {
    state_changed_ |= setBit(state_frame_.data(), 4, 0, value);
}

void Climate::setFanSpeed(uint8_t value) {
    if (state_frame_.data()[1] != value) {
        state_frame_.data()[1] = value;
        state_changed_ = true;
    }
}

void Climate::setDriverTemp(uint8_t value) {
    if (state_frame_.data()[2] != value) {
        state_frame_.data()[2] = value;
        state_changed_ = true;
    }
}

void Climate::setPassengerTemp(uint8_t value) {
    if (state_frame_.data()[3] != value) {
        state_frame_.data()[3] = value;
        state_changed_ = true;
    }
}

void Climate::setOutsideTemp(uint8_t value) {
    if (state_frame_.data()[7] != value) {
        state_frame_.data()[7] = value;
        state_changed_ = true;
    }
}

void Climate::setMode(NissanR51::ClimateVents vents) {
    switch (vents) {
        case NissanR51::CLIMATE_VENTS_FACE:
            setFace(true);
            setFeet(false);
            setFrontDefrost(false);
            break;
        case NissanR51::CLIMATE_VENTS_FACE_FEET:
            setFace(true);
            setFeet(true);
            setFrontDefrost(false);
            break;
        case NissanR51::CLIMATE_VENTS_FEET:
            setFace(false);
            setFeet(true);
            setFrontDefrost(false);
            break;
        case NissanR51::CLIMATE_VENTS_FEET_WINDSHIELD:
            setFace(false);
            setFeet(true);
            setFrontDefrost(true);
            break;
        case NissanR51::CLIMATE_VENTS_WINDSHIELD:
            setFace(false);
            setFeet(false);
            setFrontDefrost(true);
            break;
        default:
            setFace(false);
            setFeet(false);
            setFrontDefrost(false);
            break;
    }
}
