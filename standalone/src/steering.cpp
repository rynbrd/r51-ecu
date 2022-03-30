#include "steering.h"

#include <AFake.h>
#include "binary.h"
#include "config.h"
#include "debug.h"


// This needs to be placed in memory.
static constexpr const int kSteeringKeypadValues[] = STEERING_SWITCH_VALUES;
static constexpr const uint32_t kSteeringKeypadHeartbeat = 500;

SteeringKeypad::SteeringKeypad(AFake::Clock* clock, AFake::GPIO* gpio) : last_change_(0), clock_(clock) {
    initFrame(&frame_, STEERING_SWITCH_FRAME_ID, STEERING_SWITCH_FRAME_LEN);
    sw_a_ = new AnalogMultiButton(
            STEERING_SWITCH_A_PIN, STEERING_SWITCH_COUNT, kSteeringKeypadValues,
            AnalogMultiButton::DEFAULT_DEBOUNCE_DURATION,
            AnalogMultiButton::DEFAULT_ANALOG_RESOLUTION,
            clock, gpio);
    sw_b_ = new AnalogMultiButton(
            STEERING_SWITCH_B_PIN, STEERING_SWITCH_COUNT, kSteeringKeypadValues,
            AnalogMultiButton::DEFAULT_DEBOUNCE_DURATION,
            AnalogMultiButton::DEFAULT_ANALOG_RESOLUTION,
            clock, gpio);
}

SteeringKeypad::~SteeringKeypad() {
    delete sw_a_;
    delete sw_b_;
}

void SteeringKeypad::receive(const Broadcast& broadcast) {
    sw_a_->update();
    bool changed = false;
    if (sw_a_->onPress(0))  {
        // power pressed
        setBit(frame_.data, 0, 0, 1);
        changed = true;
        INFO_MSG("steering: press power");
    } else if (sw_a_->onPress(1)) {
        // seek down pressed
        setBit(frame_.data, 0, 5, 1);
        changed = true;
        INFO_MSG("steering: press seek down");
    } else if (sw_a_->onPress(2)) {
        // volume down pressed
        setBit(frame_.data, 0, 3, 1);
        changed = true;
        INFO_MSG("steering: press volume down");
    } else if (sw_a_->onRelease(0))  {
        // power released
        setBit(frame_.data, 0, 0, 0);
        changed = true;
        INFO_MSG("steering: release power");
    } else if (sw_a_->onRelease(1)) {
        // seek down released
        setBit(frame_.data, 0, 5, 0);
        changed = true;
        INFO_MSG("steering: release seek down");
    } else if (sw_a_->onRelease(2)) {
        // volume down released
        setBit(frame_.data, 0, 3, 0);
        changed = true;
        INFO_MSG("steering: release volume down");
    }

    sw_b_->update();
    if (sw_b_->onPress(0))  {
        // mode pressed
        setBit(frame_.data, 0, 1, 1);
        changed = true;
        INFO_MSG("steering: press mode");
    } else if (sw_b_->onPress(1)) {
        // seek up pressed
        setBit(frame_.data, 0, 4, 1);
        changed = true;
        INFO_MSG("steering: press seek up");
    } else if (sw_b_->onPress(2)) {
        // volume up pressed
        setBit(frame_.data, 0, 2, 1);
        changed = true;
        INFO_MSG("steering: press volume up");
    } else if (sw_b_->onRelease(0))  {
        // mode released
        setBit(frame_.data, 0, 1, 0);
        changed = true;
        INFO_MSG("steering: release mode");
    } else if (sw_b_->onRelease(1)) {
        // seek up released
        setBit(frame_.data, 0, 4, 0);
        changed = true;
        INFO_MSG("steering: release seek up");
    } else if (sw_b_->onRelease(2)) {
        // volume up released
        setBit(frame_.data, 0, 2, 0);
        changed = true;
        INFO_MSG("steering: release volume up");
    }

    if (changed || clock_->millis() - last_change_ >= STEERING_SWITCH_FRAME_HB) {
        last_change_ = clock_->millis();
        broadcast(frame_);
    }
}
