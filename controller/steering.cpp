#include "steering.h"

// This needs to be put into memory.
static constexpr const int kSteeringSwitchValues[] = STEERING_SWITCH_VALUES;

SteeringSwitch::SteeringSwitch(int sw_a_pin, int sw_b_pin) {
    sw_a_ = new AnalogMultiButton(sw_a_pin, STEERING_SWITCH_COUNT, kSteeringSwitchValues);
    sw_b_ = new AnalogMultiButton(sw_b_pin, STEERING_SWITCH_COUNT, kSteeringSwitchValues);
}

SteeringSwitch::~SteeringSwitch() {
    delete sw_a_;
    delete sw_b_;
}

void SteeringSwitch::connect(KeypadController* controller) {
    controller_ = controller;
}

void SteeringSwitch::read() {
    if (controller_ == nullptr) {
        return;
    }

    sw_a_->update();
    if (sw_a_->onPress(0))  {
        controller_->press(KeypadController::POWER);
    } else if (sw_a_->onPress(1)) {
        controller_->press(KeypadController::SEEK_DOWN);
    } else if (sw_a_->onPress(2)) {
        controller_->press(KeypadController::VOLUME_DOWN);
    } else if (sw_a_->onRelease(0))  {
        controller_->release(KeypadController::POWER);
    } else if (sw_a_->onRelease(1)) {
        controller_->release(KeypadController::SEEK_DOWN);
    } else if (sw_a_->onRelease(2)) {
        controller_->release(KeypadController::VOLUME_DOWN);
    }

    sw_b_->update();
    if (sw_b_->onPress(0))  {
        controller_->press(KeypadController::MODE);
    } else if (sw_b_->onPress(1)) {
        controller_->press(KeypadController::SEEK_UP);
    } else if (sw_b_->onPress(2)) {
        controller_->press(KeypadController::VOLUME_UP);
    } else if (sw_b_->onRelease(0))  {
        controller_->release(KeypadController::MODE);
    } else if (sw_b_->onRelease(1)) {
        controller_->release(KeypadController::SEEK_UP);
    } else if (sw_b_->onRelease(2)) {
        controller_->release(KeypadController::VOLUME_UP);
    }
}
