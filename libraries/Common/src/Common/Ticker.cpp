#include "Ticker.h"

namespace R51 {

Ticker::Ticker(uint32_t interval, Faker::Clock* clock) :
        interval_(interval), last_tick_(0), clock_(clock) {}

bool Ticker::active() const {
    if (interval_ == 0) {
        return false;
    }
    if (clock_->millis() - last_tick_ >= interval_) {
        return true;
    }
    return false;
}

void Ticker::reset() {
    last_tick_ = clock_->millis();
}

void Ticker::reset(uint32_t interval) {
    interval_ = interval;
    reset();
}

}  // namespace R51
