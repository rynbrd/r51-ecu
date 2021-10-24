#include "climate.h"

#include <Arduino.h>
#include "debug.h"
#include "binary.h"


ClimateController::ClimateController() {
    unit_online_ = false;

    // Send heartbeat every 100ms during handshake.
    heartbeat_delay_ = 100;
    last_heartbeat_ = 0;

    // Set control frames to send handshake.
    memset(frame540_, 0, 8);
    memset(frame541_, 0, 8);
    frame540_[0] = 0x80;
    frame541_[1] = 0x80;
    state_changed_ = true;
}

bool ClimateController::online() const {
    return frame540_[0] == 0x60;
}

void ClimateController::deactivate() {
    if (!online()) {
        return;
    }
    toggleBit(frame540_, 6, 7);
    state_changed_ = true;
}

void ClimateController::toggleAuto() {
    toggle(frame540_, 6, 5);
}

void ClimateController::toggleAc() {
    toggle(frame540_, 5, 3);
}

void ClimateController::toggleDual() {
    toggle(frame540_, 6, 3);
}

void ClimateController::toggleRecirculate() {
    toggle(frame541_, 1, 6);
}


void ClimateController::cycleMode() {
    toggle(frame540_, 6, 0);
}

void ClimateController::toggleFrontDefrost() {
    toggle(frame540_, 6, 1);
}

void ClimateController::toggleRearDefrost() {
    // rear defrost control signal tbd
}

void ClimateController::increaseFanSpeed() {
    toggle(frame541_, 0, 5);
}

void ClimateController::decreaseFanSpeed() {
    toggle(frame541_, 0, 4);
}

void ClimateController::setDriverTemp(uint8_t temp) {
    if (toggle(frame540_, 5, 5)) {
        frame540_[3] = temp + 0xB8;
    }
}

void ClimateController::setPassengerTemp(uint8_t temp) {
    if (toggle(frame540_, 5, 5)) {
        frame540_[4] = temp - 0x33;
    }
}

void ClimateController::update(uint32_t id, uint8_t len, byte* data) {
    // Ignore invalid frames.
    if (id != 0x54B || len < 8) {
        return;
    }

    // Already online, nothing more to do.
    if (unit_online_) {
        return;
    }

    unit_online_ = data[0] & 0xA0 != 0x20;
    if (unit_online_) {
        // Unit transitions to online mode. Send empty control frames to ack.
        // Increase heartbeat time to 200ms.
        unit_online_ = true;
        frame540_[0] = 0;
        frame540_[0] = 0;
        state_changed_ = true;
        heartbeat_delay_ = 200;
        Debug.println("climate system reports online"); 
    }
}

void ClimateController::emit(MCP_CAN* can) {
    // Heartbeat control frames at least every 200ms to keep the A/C Auto Amp
    // alive.
    if (millis() - last_heartbeat_ > heartbeat_delay_) {
        state_changed_ = true;
    }

    if (state_changed_) {
        Debug.print("send ");
        Debug.println(0x540, 8, frame540_);
        if (can->sendMsgBuf(0x540, false, 8, frame540_) != CAN_OK) {
            Debug.println("error sending climate control frame 540");
        }
        Debug.print("send ");
        Debug.println(0x541, 8, frame540_);
        if (can->sendMsgBuf(0x541, false, 8, frame541_) != CAN_OK) {
            Debug.println("error sending climate control frame 541");
        }

        if (unit_online_ && frame540_[0] == 0x00) {
            // Fill in control frames for regular operation after the ack is
            // sent.
            frame540_[0] = 0x60;
            frame540_[1] = 0x40;
            frame540_[6] = 0x04;
            Debug.println("climate system fully operational");
        }

        state_changed_ = false;
        last_heartbeat_ = millis();
    }
}

bool ClimateController::toggle(byte* frame, uint8_t offset, uint8_t bit) {
    if (!online()) {
        return false;
    }
    toggleBit(frame, offset, bit);
    state_changed_ = true;
    return true;
}
