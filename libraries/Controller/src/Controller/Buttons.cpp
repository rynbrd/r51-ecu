#include "Buttons.h"

#include <Arduino.h>

namespace R51 {

void RepeatButton::press() {
    ticker_.resume();
}

bool RepeatButton::trigger() {
    if (ticker_.active()) {
        ticker_.reset();
        return true;
    }
    return false;
}

bool RepeatButton::release() {
    ticker_.pause();
    return !ticker_.triggered();
}

void LongPressButton::press() {
    pressed_ = clock_->millis();
    state_ = 1;
}

bool LongPressButton::trigger() {
    if (state_ == 1 && clock_->millis() - pressed_ >= timeout_) {
        state_ = 2;
        return true;
    }
    return false;
}

bool LongPressButton::release() {
    bool ret = state_ == 1;
    state_ = 0;
    return ret;
}

}  // namespace R51
