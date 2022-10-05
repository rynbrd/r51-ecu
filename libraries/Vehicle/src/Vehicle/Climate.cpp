#include "Climate.h"

#include <Canny.h>
#include <Common.h>
#include <Faker.h>
#include <Foundation.h>
#include "Units.h"

namespace R51 {

enum InitState : uint8_t {
    INIT_TEMP = 0x01,
    INIT_SYSTEM = 0x02,
};

enum AirflowMode : uint8_t {
    AIRFLOW_OFF = 0x00,
    AIRFLOW_FACE = 0x04,
    AIRFLOW_FACE_FEET = 0x08,
    AIRFLOW_FEET = 0x0C,
    AIRFLOW_FEET_WINDSHIELD = 0x10,
    AIRFLOW_WINDSHIELD = 0x34,
    AIRFLOW_AUTO_FACE = 0x84,
    AIRFLOW_AUTO_FACE_FEET = 0x88,
    AIRFLOW_AUTO_FEET = 0x8C,
};

#define CONTROL_INIT_EXPIRE 400
#define CONTROL_INIT_TICK 100
#define CONTROL_FRAME_TICK 200

Climate::Climate(uint32_t tick_ms, Faker::Clock* clock) :
    clock_(clock), startup_(0),
    state_ticker_(tick_ms, clock), control_ticker_(CONTROL_INIT_TICK, clock),
    state_request_(false), state_init_(0), control_init_(false) {}

void Climate::handle(const Message& msg, const Caster::Yield<Message>& yield) {
    //TODO: Emit events directly.
    switch (msg.type()) {
        case Message::CAN_FRAME:
            handleSystemFrame(msg.can_frame(), yield);
            handleTempFrame(msg.can_frame(), yield);
            break;
        case Message::EVENT:
            handleEvent(msg.event(), yield);
            break;
        default:
            break;
    }
}

void Climate::handleTempFrame(const Canny::Frame& frame, const Caster::Yield<Message>& yield) {
    if (frame.id() != 0x54A || frame.size() < 8) {
        return;
    }

    if (temp_state_.driver_temp(frame.data()[4]) |
        temp_state_.passenger_temp(frame.data()[5]) |
        temp_state_.outside_temp(frame.data()[7]) |
        temp_state_.units(frame.data()[3] == 0x40 ? UNITS_METRIC : UNITS_US)) {
        yield(temp_state_);
    }
}

void Climate::handleSystemFrame(const Canny::Frame& frame, const Caster::Yield<Message>& yield) {
    if (frame.id() != 0x54B || frame.size() < 8) {
        return;
    }

    bool airflow_state_changed = (
        airflow_state_.fan_speed((frame.data()[2] + 1) / 2) |
        airflow_state_.recirculate(getBit(frame.data(), 3, 4)));

    switch((AirflowMode)frame.data()[1]) {
        case AIRFLOW_OFF:
            airflow_state_changed |= (
                airflow_state_.face(false) |
                airflow_state_.feet(false) |
                airflow_state_.windshield(false));
            break;
        case AIRFLOW_FACE:
        case AIRFLOW_AUTO_FACE:
            airflow_state_changed |= (
                airflow_state_.face(true) |
                airflow_state_.feet(false) |
                airflow_state_.windshield(false));
            break;
        case AIRFLOW_FACE_FEET:
        case AIRFLOW_AUTO_FACE_FEET:
            airflow_state_changed |= (
                airflow_state_.face(true) |
                airflow_state_.feet(true) |
                airflow_state_.windshield(false));
            break;
        case AIRFLOW_FEET:
        case AIRFLOW_AUTO_FEET:
            airflow_state_changed |= (
                airflow_state_.face(false) |
                airflow_state_.feet(true) |
                airflow_state_.windshield(false));
            break;
        case AIRFLOW_FEET_WINDSHIELD:
            airflow_state_changed |= (
                airflow_state_.face(false) |
                airflow_state_.feet(true) |
                airflow_state_.windshield(true));
            break;
        case AIRFLOW_WINDSHIELD:
            airflow_state_changed |= (
                airflow_state_.face(false) |
                airflow_state_.feet(false) |
                airflow_state_.windshield(true));
            break;
    }

    bool system_state_changed = (
        system_state_.ac(getBit(frame.data(), 0, 3)) |
        system_state_.dual(getBit(frame.data(), 3, 7)));

    if (airflow_state_.windshield()) {
        system_state_changed |= system_state_.mode(CLIMATE_SYSTEM_DEFROST);
    } else if (getBit(frame.data(), 0, 7)) {
        system_state_changed |= system_state_.mode(CLIMATE_SYSTEM_OFF);
    } else if (getBit(frame.data(), 0, 0)) {
        system_state_changed |= system_state_.mode(CLIMATE_SYSTEM_AUTO);
    } else {
        system_state_changed |= system_state_.mode(CLIMATE_SYSTEM_MANUAL);
    }

    if (system_state_changed) {
        yield(system_state_);
    }
    if (airflow_state_changed) {
        yield(airflow_state_);
    }
}

void Climate::handleEvent(const Event& event, const Caster::Yield<Message>& yield) {
    if (event.subsystem != (uint8_t)SubSystem::CLIMATE) {
        return;
    }

    bool system_control_changed = false;
    bool fan_control_changed = false;
    switch ((ClimateEvent)event.id) {
        case ClimateEvent::REQUEST:
            state_request_ = true;
            break;
        case ClimateEvent::TURN_OFF:
            system_control_.turnOff();
            system_control_changed = true;
            break;
        case ClimateEvent::TOGGLE_AUTO:
            system_control_.toggleAuto();
            system_control_changed = true;
            break;
        case ClimateEvent::TOGGLE_AC:
            system_control_.toggleAC();
            system_control_changed = true;
            break;
        case ClimateEvent::TOGGLE_DUAL:
            system_control_.toggleDual();
            system_control_changed = true;
            break;
        case ClimateEvent::TOGGLE_DEFROST:
            system_control_.toggleDefrost();
            system_control_changed = true;
            break;
        case ClimateEvent::INC_FAN_SPEED:
            fan_control_.incFanSpeed();
            fan_control_changed = true;
            break;
        case ClimateEvent::DEC_FAN_SPEED:
            fan_control_.decFanSpeed();
            fan_control_changed = true;
            break;
        case ClimateEvent::TOGGLE_RECIRCULATE:
            fan_control_.toggleRecirculate();
            fan_control_changed = true;
            break;
        case ClimateEvent::CYCLE_AIRFLOW_MODE:
            system_control_.cycleMode();
            system_control_changed = true;
            break;
        case ClimateEvent::INC_DRIVER_TEMP:
            if (system_state_.mode() != CLIMATE_SYSTEM_OFF) {
                system_control_.incDriverTemp();
                system_control_changed = true;
            }
            break;
        case ClimateEvent::DEC_DRIVER_TEMP:
            if (system_state_.mode() != CLIMATE_SYSTEM_OFF) {
                system_control_.decDriverTemp();
                system_control_changed = true;
            }
            break;
        case ClimateEvent::INC_PASSENGER_TEMP:
            if (system_state_.mode() != CLIMATE_SYSTEM_OFF) {
                system_control_.incPassengerTemp();
                system_control_changed = true;
            }
            break;
        case ClimateEvent::DEC_PASSENGER_TEMP:
            if (system_state_.mode() != CLIMATE_SYSTEM_OFF) {
                system_control_.decPassengerTemp();
                system_control_changed = true;
            }
            break;
        default:
            break;
    }

    if (system_control_changed) {
        yield(system_control_);
    }
    if (fan_control_changed) {
        yield(fan_control_);
    }
}

void Climate::emit(const Caster::Yield<Message>& yield) {
    if (startup_ == 0) {
        startup_ = clock_->millis();
    }
    if (!control_init_ && clock_->millis() - startup_ >= CONTROL_INIT_EXPIRE) {
        system_control_.ready();
        fan_control_.ready();
        yield(system_control_);
        yield(fan_control_);
        control_ticker_.reset(CONTROL_FRAME_TICK);
        control_init_ = true;
    }

    if (control_ticker_.active()) {
        yield(system_control_);
        yield(fan_control_);
        control_ticker_.reset();
    }

    if (state_request_ || state_ticker_.active()) {
        yield(temp_state_);
        yield(system_state_);
        yield(airflow_state_);
        state_ticker_.reset();
        state_request_ = false;
    }
}

}  // namespace R51
