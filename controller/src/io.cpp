#include "io.h"

void MomentaryPin::update() {
    if (triggered_) {
        if (clock_->millis() - trigger_time_ >= trigger_ms_ + cooldown_ms_) {
            triggered_ = false;
        } else if (clock_->millis() - trigger_time_ >= trigger_ms_) {
            gpio_->digitalWrite(pin_, !high_);
        }
    }
}

// Trigger the pin. Return true on success or false if the pin is
// currently triggered or in the cooldown phase.
bool MomentaryPin::trigger() {
    if (triggered_) {
        return false;
    }

    gpio_->digitalWrite(pin_, high_);
    triggered_ = true;
    trigger_time_ = clock_->millis();
    return true;
}
