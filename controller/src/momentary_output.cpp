#include "momentary_output.h"


MomentaryOutput::MomentaryOutput(int pin, uint16_t trigger_ms, int32_t cooldown_ms, bool high,
        Clock* clock, GPIO* gpio)
    : clock_(clock), gpio_(gpio),
      pin_(pin), high_(high), triggered_(false), 
      trigger_ms_(trigger_ms), trigger_time_(0),
      cooldown_ms_(cooldown_ms < 0 ? trigger_ms : (uint16_t)cooldown_ms) {
    gpio_->pinMode(pin_, OUTPUT);
    gpio_->digitalWrite(pin_, high_ ? LOW : HIGH);
}

MomentaryOutput::MomentaryOutput(int pin, uint16_t trigger_ms, Clock* clock, GPIO* gpio)
    : MomentaryOutput(pin, trigger_ms, -1, true, clock, gpio) {}

void MomentaryOutput::update() {
    if (triggered_) {
        if (clock_->millis() - trigger_time_ >= trigger_ms_ + cooldown_ms_) {
            triggered_ = false;
        } else if (clock_->millis() - trigger_time_ >= trigger_ms_) {
            gpio_->digitalWrite(pin_, high_ ? LOW : HIGH);
        }
    }
}

// Trigger the pin. Return true on success or false if the pin is
// currently triggered or in the cooldown phase.
bool MomentaryOutput::trigger() {
    if (triggered_) {
        return false;
    }

    gpio_->digitalWrite(pin_, high_ ? HIGH : LOW);
    triggered_ = true;
    trigger_time_ = clock_->millis();
    return true;
}
