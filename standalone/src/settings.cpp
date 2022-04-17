#include "settings.h"

#include <Canny.h>
#include <Faker.h>

#include "binary.h"
#include "bus.h"
#include "config.h"
#include "debug.h"


Settings::Settings(Faker::Clock* clock) :
        clock_(clock), state_changed_(false), state_last_broadcast_(0),
        state_(SETTINGS_STATE_FRAME_ID, 0, 8), settings_(clock) {
    memset(control_state_, 0, 8);
}

void Settings::receive(const Broadcast& broadcast) {
    while (settings_.available()) {
        broadcast(settings_.frame());
    }
    if (state_changed_ || clock_->millis() - state_last_broadcast_ >= SETTINGS_STATE_FRAME_HB) {
        broadcast(state_);
        state_changed_ = false;
        state_last_broadcast_ = clock_->millis();
    }
}

void Settings::send(const Canny::Frame& frame) {
    if (frame.size() < 8) {
        return;
    }
    if (frame.id() == SETTINGS_CONTROL_FRAME_ID) {
        handleControl(frame);
    } else {
        state_changed_ = settings_.handle(frame);
    }
}

void Settings::handleControl(const Canny::Frame& frame) {
    // check if any bits have flipped
    if (xorBits(control_state_, frame.data(), 0, 0)) {
        settings_.toggleAutoInteriorIllumination();
    } else if (xorBits(control_state_, frame.data(), 0, 1)) {
        settings_.toggleSlideDriverSeatBackOnExit();
    } else if (xorBits(control_state_, frame.data(), 0, 2)) {
        settings_.toggleSpeedSensingWiperInterval();
    } else if (xorBits(control_state_, frame.data(), 1, 0)) {
        settings_.nextAutoHeadlightSensitivity();
    } else if (xorBits(control_state_, frame.data(), 1, 1)) {
        settings_.prevAutoHeadlightSensitivity();
    } else if (xorBits(control_state_, frame.data(), 1, 4)) {
        settings_.nextAutoHeadlightOffDelay();
    } else if (xorBits(control_state_, frame.data(), 1, 5)) {
        settings_.prevAutoHeadlightOffDelay();
    } else if (xorBits(control_state_, frame.data(), 2, 0)) {
        settings_.toggleSelectiveDoorUnlock();
    } else if (xorBits(control_state_, frame.data(), 2, 4)) {
        settings_.nextAutoReLockTime();
    } else if (xorBits(control_state_, frame.data(), 2, 5)) {
        settings_.prevAutoReLockTime();
    } else if (xorBits(control_state_, frame.data(), 3, 0)) {
        settings_.toggleRemoteKeyResponseHorn();
    } else if (xorBits(control_state_, frame.data(), 3, 2)) {
        settings_.nextRemoteKeyResponseLights();
    } else if (xorBits(control_state_, frame.data(), 3, 3)) {
        settings_.prevRemoteKeyResponseLights();
    } else if (xorBits(control_state_, frame.data(), 7, 0)) {
        settings_.retrieveSettings();
    } else if (xorBits(control_state_, frame.data(), 7, 7)) {
        settings_.resetSettingsToDefault();
    }

    // update the stored control state
    memcpy(control_state_, frame.data(), 8);
}

bool Settings::filter(const Canny::Frame& frame) const {
    return frame.id() == SETTINGS_CONTROL_FRAME_ID ||
        frame.id() == 0x72E ||
        frame.id() == 0x72F;
}

bool Settings::init() {
    return settings_.init();
}
