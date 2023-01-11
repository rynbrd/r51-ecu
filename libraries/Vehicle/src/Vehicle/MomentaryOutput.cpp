#include "MomentaryOutput.h"

#include <Faker.h>

namespace R51 {

MomentaryOutput::MomentaryOutput(int pin, uint16_t trigger_ms, int32_t cooldown_ms, Mode mode,
        Faker::Clock* clock, Faker::GPIO* gpio)
    : clock_(clock), gpio_(gpio),
      pin_(pin), mode_(mode), triggered_(0), 
      trigger_ms_(trigger_ms), trigger_time_(0),
      cooldown_ms_(cooldown_ms < 0 ? trigger_ms : (uint16_t)cooldown_ms) {}

void MomentaryOutput::init() {
    gpio_->pinMode(pin_, OUTPUT);
    switch (mode_) {
        case MOMENTARILY_HIGH:
            gpio_->digitalWrite(pin_, HIGH);
            break;
        case MOMENTARILY_LOW:
            gpio_->pinMode(pin_, OUTPUT);
            gpio_->digitalWrite(pin_, LOW);
            break;
        case MOMENTARILY_DRAIN:
            Serial.println("output: high impedence");
            gpio_->digitalWrite(pin_, HIGH);
            gpio_->pinMode(pin_, INPUT);
            break;
    }
}

MomentaryOutput::MomentaryOutput(int pin, uint16_t trigger_ms, Faker::Clock* clock, Faker::GPIO* gpio)
    : MomentaryOutput(pin, trigger_ms, -1, MOMENTARILY_HIGH, clock, gpio) {}

void MomentaryOutput::update() {
    if (triggered_) {
        if (clock_->millis() - trigger_time_ >= trigger_ms_ + cooldown_ms_) {
            // cooldown expired, reset trigger
            triggered_ = 0;
        } else if (triggered_ < 2 && clock_->millis() - trigger_time_ >= trigger_ms_) {
            // timer expired, turn pin "off"
            triggered_ = 2;
            switch (mode_) {
                case MOMENTARILY_HIGH:
                    gpio_->digitalWrite(pin_, LOW);
                    break;
                case MOMENTARILY_LOW:
                    gpio_->digitalWrite(pin_, HIGH);
                    break;
                case MOMENTARILY_DRAIN:
                    Serial.println("output: high impedence");
                    gpio_->digitalWrite(pin_, HIGH);
                    gpio_->pinMode(pin_, INPUT);
            }
        }
    }
}

// Trigger the pin. Return true on success or false if the pin is
// currently triggered or in the cooldown phase.
bool MomentaryOutput::trigger() {
    if (triggered_) {
        // pin already "on" or cooling down
        return false;
    }

    // turn pin "on"
    switch (mode_) {
        case MOMENTARILY_HIGH:
            gpio_->digitalWrite(pin_, HIGH);
            break;
        case MOMENTARILY_LOW:
            gpio_->digitalWrite(pin_, LOW);
            break;
        case MOMENTARILY_DRAIN:
            Serial.println("output: open drain");
            gpio_->pinMode(pin_, OUTPUT);
            gpio_->digitalWrite(pin_, LOW);
            break;
    }
    triggered_ = 1;
    trigger_time_ = clock_->millis();
    return true;
}

}  // namespace R51
