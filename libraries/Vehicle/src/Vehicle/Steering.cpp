#include "Steering.h"

namespace R51 {

// Steering keypad button count and resistance thresholds.
static const int kSteeringKeypadCount = 3;
static constexpr const int kSteeringKeypadValues[] = {50, 280, 640};

SteeringKeypad::SteeringKeypad(int sw_a_pin, int sw_b_pin, Faker::Clock* clock, Faker::GPIO* gpio) :
        sw_a_(
            sw_a_pin, kSteeringKeypadCount, kSteeringKeypadValues,
            AnalogMultiButton::DEFAULT_DEBOUNCE_DURATION,
            AnalogMultiButton::DEFAULT_ANALOG_RESOLUTION,
            clock, gpio),
        sw_b_(
            sw_b_pin, kSteeringKeypadCount, kSteeringKeypadValues,
            AnalogMultiButton::DEFAULT_DEBOUNCE_DURATION,
            AnalogMultiButton::DEFAULT_ANALOG_RESOLUTION,
            clock, gpio) {}

void SteeringKeypad::emit(const Caster::Yield<Message>& yield) {
    Event event;
    sw_a_.update();
    if (sw_a_.onPress(0))  {
        event = Event((uint8_t)SubSystem::STEERING_KEYPAD, (uint8_t)SteeringKeypadEvent::POWER, {0x01});
    } else if (sw_a_.onPress(1)) {
        event = Event((uint8_t)SubSystem::STEERING_KEYPAD, (uint8_t)SteeringKeypadEvent::SEEK_DOWN, {0x01});
    } else if (sw_a_.onPress(2)) {
        event = Event((uint8_t)SubSystem::STEERING_KEYPAD, (uint8_t)SteeringKeypadEvent::VOLUME_DOWN, {0x01});
    } else if (sw_a_.onRelease(0))  {
        event = Event((uint8_t)SubSystem::STEERING_KEYPAD, (uint8_t)SteeringKeypadEvent::POWER, {0x00});
    } else if (sw_a_.onRelease(1)) {
        event = Event((uint8_t)SubSystem::STEERING_KEYPAD, (uint8_t)SteeringKeypadEvent::SEEK_DOWN, {0x00});
    } else if (sw_a_.onRelease(2)) {
        event = Event((uint8_t)SubSystem::STEERING_KEYPAD, (uint8_t)SteeringKeypadEvent::VOLUME_DOWN, {0x00});
    }
    if (event.subsystem != 0) {
        yield(event);
    }

    event.subsystem = 0;
    sw_b_.update();
    if (sw_b_.onPress(0))  {
        event = Event((uint8_t)SubSystem::STEERING_KEYPAD, (uint8_t)SteeringKeypadEvent::MODE, {0x01});
    } else if (sw_b_.onPress(1)) {
        event = Event((uint8_t)SubSystem::STEERING_KEYPAD, (uint8_t)SteeringKeypadEvent::SEEK_UP, {0x01});
    } else if (sw_b_.onPress(2)) {
        event = Event((uint8_t)SubSystem::STEERING_KEYPAD, (uint8_t)SteeringKeypadEvent::VOLUME_UP, {0x01});
    } else if (sw_b_.onRelease(0))  {
        event = Event((uint8_t)SubSystem::STEERING_KEYPAD, (uint8_t)SteeringKeypadEvent::MODE, {0x00});
    } else if (sw_b_.onRelease(1)) {
        event = Event((uint8_t)SubSystem::STEERING_KEYPAD, (uint8_t)SteeringKeypadEvent::SEEK_UP, {0x00});
    } else if (sw_b_.onRelease(2)) {
        event = Event((uint8_t)SubSystem::STEERING_KEYPAD, (uint8_t)SteeringKeypadEvent::VOLUME_UP, {0x00});
    }

    if (event.subsystem != 0) {
        yield(event);
    }
}

}  // namespace R51
