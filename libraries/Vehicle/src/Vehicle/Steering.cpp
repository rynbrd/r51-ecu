#include "Steering.h"

#include <Arduino.h>
#include <Caster.h>
#include <Faker.h>

namespace R51 {

// Steering keypad button count and resistance thresholds.
// Values are set for use with a 2k pull-up resistor.
static constexpr const int kSteeringKeypadValues[] = {20, 332, 1016};
static const int kSteeringKeypadCount = 3;

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
        key_(keypad), clock_(clock), gpio_(gpio),
        a_pin_(sw_a_pin), b_pin_(sw_b_pin) {}

void SteeringKeypad::begin() {
    // Initialize GPIOs and discard the first read.
    gpio_->pinMode(a_pin_, INPUT);
    gpio_->digitalWrite(a_pin_, LOW);
    gpio_->pinMode(b_pin_, INPUT);
    gpio_->digitalWrite(b_pin_, LOW);
    clock_->delay(10);
    gpio_->analogRead(a_pin_);
    gpio_->analogRead(b_pin_);
}

void SteeringKeypad::emit(const Caster::Yield<Message>& yield) {
    sw_a_.update();
    if (sw_a_.onPress(0))  {
        key_.key((uint8_t)SteeringKey::POWER);
        key_.pressed(true);
        yield(MessageView(&key_));
    } else if (sw_a_.onPress(1)) {
        key_.key((uint8_t)SteeringKey::SEEK_UP);
        key_.pressed(true);
        yield(MessageView(&key_));
    } else if (sw_a_.onPress(2)) {
        key_.key((uint8_t)SteeringKey::VOLUME_UP);
        key_.pressed(true);
        yield(MessageView(&key_));
    } else if (sw_a_.onRelease(0))  {
        key_.key((uint8_t)SteeringKey::POWER);
        key_.pressed(false);
        yield(MessageView(&key_));
    } else if (sw_a_.onRelease(1)) {
        key_.key((uint8_t)SteeringKey::SEEK_UP);
        key_.pressed(false);
        yield(MessageView(&key_));
    } else if (sw_a_.onRelease(2)) {
        key_.key((uint8_t)SteeringKey::VOLUME_UP);
        key_.pressed(false);
        yield(MessageView(&key_));
    }

    sw_b_.update();
    if (sw_b_.onPress(0))  {
        key_.key((uint8_t)SteeringKey::MODE);
        key_.pressed(true);
        yield(MessageView(&key_));
    } else if (sw_b_.onPress(1)) {
        key_.key((uint8_t)SteeringKey::SEEK_DOWN);
        key_.pressed(true);
        yield(MessageView(&key_));
    } else if (sw_b_.onPress(2)) {
        key_.key((uint8_t)SteeringKey::VOLUME_DOWN);
        key_.pressed(true);
        yield(MessageView(&key_));
    } else if (sw_b_.onRelease(0))  {
        key_.key((uint8_t)SteeringKey::MODE);
        key_.pressed(false);
        yield(MessageView(&key_));
    } else if (sw_b_.onRelease(1)) {
        key_.key((uint8_t)SteeringKey::SEEK_DOWN);
        key_.pressed(false);
        yield(MessageView(&key_));
    } else if (sw_b_.onRelease(2)) {
        key_.key((uint8_t)SteeringKey::VOLUME_DOWN);
        key_.pressed(false);
        yield(MessageView(&key_));
    }
}

}  // namespace R51
