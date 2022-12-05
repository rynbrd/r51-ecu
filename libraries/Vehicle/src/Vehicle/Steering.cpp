#include "Steering.h"

#include <Arduino.h>
#include <Caster.h>
#include <Faker.h>

namespace R51 {

// Steering keypad button count and resistance thresholds.
static const int kSteeringKeypadCount = 3;
static constexpr const int kSteeringKeypadValues[] = {50, 280, 640};

SteeringKeypad::SteeringKeypad(uint8_t keypad, int sw_a_pin, int sw_b_pin,
    Faker::Clock* clock, Faker::GPIO* gpio) :
        sw_a_(
            sw_a_pin, kSteeringKeypadCount, kSteeringKeypadValues,
            AnalogMultiButton::DEFAULT_DEBOUNCE_DURATION,
            AnalogMultiButton::DEFAULT_ANALOG_RESOLUTION,
            clock, gpio),
        sw_b_(
            sw_b_pin, kSteeringKeypadCount, kSteeringKeypadValues,
            AnalogMultiButton::DEFAULT_DEBOUNCE_DURATION,
            AnalogMultiButton::DEFAULT_ANALOG_RESOLUTION,
            clock, gpio),
        key_(keypad) {}

void SteeringKeypad::emit(const Caster::Yield<Message>& yield) {
    sw_a_.update();
    if (sw_a_.onPress(0))  {
        key_.key((uint8_t)SteeringKey::POWER);
        key_.pressed(true);
        yield(&key_);
    } else if (sw_a_.onPress(1)) {
        key_.key((uint8_t)SteeringKey::SEEK_DOWN);
        key_.pressed(true);
        yield(&key_);
    } else if (sw_a_.onPress(2)) {
        key_.key((uint8_t)SteeringKey::VOLUME_DOWN);
        key_.pressed(true);
        yield(&key_);
    } else if (sw_a_.onRelease(0))  {
        key_.key((uint8_t)SteeringKey::POWER);
        key_.pressed(false);
        yield(&key_);
    } else if (sw_a_.onRelease(1)) {
        key_.key((uint8_t)SteeringKey::SEEK_DOWN);
        key_.pressed(false);
        yield(&key_);
    } else if (sw_a_.onRelease(2)) {
        key_.key((uint8_t)SteeringKey::VOLUME_DOWN);
        key_.pressed(false);
        yield(&key_);
    }

    sw_b_.update();
    if (sw_b_.onPress(0))  {
        key_.key((uint8_t)SteeringKey::MODE);
        key_.pressed(true);
        yield(&key_);
    } else if (sw_b_.onPress(1)) {
        key_.key((uint8_t)SteeringKey::SEEK_UP);
        key_.pressed(true);
        yield(&key_);
    } else if (sw_b_.onPress(2)) {
        key_.key((uint8_t)SteeringKey::VOLUME_UP);
        key_.pressed(true);
        yield(&key_);
    } else if (sw_b_.onRelease(0))  {
        key_.key((uint8_t)SteeringKey::MODE);
        key_.pressed(false);
        yield(&key_);
    } else if (sw_b_.onRelease(1)) {
        key_.key((uint8_t)SteeringKey::SEEK_UP);
        key_.pressed(false);
        yield(&key_);
    } else if (sw_b_.onRelease(2)) {
        key_.key((uint8_t)SteeringKey::VOLUME_UP);
        key_.pressed(false);
        yield(&key_);
    }
}

}  // namespace R51
