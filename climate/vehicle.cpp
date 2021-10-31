#include "vehicle.h"

#include <Arduino.h>
#include "binary.h"
#include "dash.h"
#include "debug.h"
#include "mcp_can.h"


VehicleController::VehicleController() {
    can_ = nullptr;
    climate_online_ = false;

    // Send heartbeat every 100ms during handshake.
    heartbeat_delay_ = 100;
    last_heartbeat_ = 0;

    // Set control frames to send handshake.
    memset(frame540_, 0, 8);
    memset(frame541_, 0, 8);
    frame540_[0] = 0x80;
    frame541_[1] = 0x80;
    frame54x_changed_ = true;
}

void VehicleController::connect(MCP_CAN* can) {
    can_ = can;
}

bool VehicleController::climateOnline() const {
    return frame540_[0] == 0x60;
}

void VehicleController::deactivateClimate() {
    if (toggle(frame540_, 6, 7)) {
        INFO_MSG("vehicle: climate: deactivate");
    }
}

void VehicleController::toggleClimateAuto() {
    if (toggle(frame540_, 6, 5)) {
        INFO_MSG("vehicle: climate: toggle auto");
    }
}

void VehicleController::toggleClimateAc() {
    if (toggle(frame540_, 5, 3)) {
        INFO_MSG("vehicle: climate: toggle a/c");
    }
}

void VehicleController::toggleClimateDual() {
    if (toggle(frame540_, 6, 3)) {
        INFO_MSG("vehicle: climate: toggle dual zone");
    }
}

void VehicleController::toggleClimateRecirculate() {
    if (toggle(frame541_, 1, 6)) {
        INFO_MSG("vehicle: climate: toggle recirculate");
    }
}


void VehicleController::cycleClimateMode() {
    if (toggle(frame540_, 6, 0)) {
        INFO_MSG("vehicle: climate: cycle mode");
    }
}

void VehicleController::toggleClimateFrontDefrost() {
    if (toggle(frame540_, 6, 1)) {
        INFO_MSG("vehicle: climate: toggle front defrost");
    }
}

void VehicleController::toggleClimateRearDefrost() {
    // TODO: determine rear defrost control signal
    if (climateOnline()) {
        INFO_MSG("vehicle: climate: toggle rear defrost (noop)");
    }
}

void VehicleController::toggleClimateMirrorDefrost() {
    // TODO: determine mirror defrost control signal
    if (climateOnline()) {
        INFO_MSG("vehicle: climate: toggle rear defrost (noop)");
    }
}

void VehicleController::increaseClimateFanSpeed() {
    if (toggle(frame541_, 0, 5)) {
        INFO_MSG("vehicle: climate: increase fan speed");
    }
}

void VehicleController::decreaseClimateFanSpeed() {
    if (toggle(frame541_, 0, 4)) {
        INFO_MSG("vehicle: climate: decrease fan speed");
    }
}

void VehicleController::setClimateDriverTemp(uint8_t temp) {
    if (!climateOnline()) {
        return;
    }
    if (temp < 60 || temp > 90) {
        ERROR_MSG_VAL("vehicle: climate: driver temperature out of range: ", temp);
        return;
    }
    toggleBit(frame540_, 5, 5);
    frame540_[3] = temp + 0xB8;
    frame54x_changed_ = true;
    INFO_MSG_VAL("vehicle: climate: set driver temperature to ", temp);
}

void VehicleController::setClimatePassengerTemp(uint8_t temp) {
    if (!climateOnline()) {
        return;
    }
    if (temp < 60 || temp > 90) {
        ERROR_MSG_VAL("vehicle: climate: passenger temperature out of range: ", temp);
        return;
    }
    toggleBit(frame540_, 5, 5);
    frame540_[4] = temp - 0x33;
    frame54x_changed_ = true;
    INFO_MSG_VAL("vehicle: climate: set passenger temperature to ", temp);
}

void VehicleController::receive(uint32_t id, uint8_t len, byte* data) {
    // Ignore invalid frames.
    if (id != 0x54B || len < 8) {
        return;
    }

    // Already online, nothing more to do.
    if (climate_online_) {
        return;
    }

    climate_online_ = (data[0] & 0xA0) != 0x20;
    if (climate_online_) {
        // Unit transitions to online mode. Send empty control frames to ack.
        // Increase heartbeat time to 200ms.
        climate_online_ = true;
        frame540_[0] = 0;
        frame540_[0] = 0;
        frame54x_changed_ = true;
        heartbeat_delay_ = 200;
        INFO_MSG("vehicle: climate system reports online"); 
    }
}

void VehicleController::push() {
    if (can_ == nullptr) {
        return;
    }

    // Heartbeat control frames at least every 200ms to keep the A/C Auto Amp
    // alive.
    if (millis() - last_heartbeat_ > heartbeat_delay_) {
        frame54x_changed_ = true;
    }

    if (frame54x_changed_) {
        INFO_MSG_FRAME("vehicle: send ", 0x540, 8, frame540_);
        if (can_->sendMsgBuf(0x540, false, 8, frame540_) != CAN_OK) {
            ERROR_MSG("vehicle: failed to send frame 0x540");
        }
        INFO_MSG_FRAME("vehicle: send ", 0x541, 8, frame541_);
        if (can_->sendMsgBuf(0x541, false, 8, frame541_) != CAN_OK) {
            ERROR_MSG("vehicle: failed to send frame 0x541");
        }

        if (climate_online_ && frame540_[0] == 0x00) {
            // Fill in control frames for regular operation after the ack is
            // sent.
            frame540_[0] = 0x60;
            frame540_[1] = 0x40;
            frame540_[6] = 0x04;
            INFO_MSG("vehicle: climate system handshake complete");
        }

        frame54x_changed_ = false;
        last_heartbeat_ = millis();
    }
}

bool VehicleController::toggle(byte* frame, uint8_t offset, uint8_t bit) {
    if (!climateOnline()) {
        return false;
    }
    toggleBit(frame, offset, bit);
    frame54x_changed_ = true;
    return true;
}

void VehicleListener::connect(DashController* dash) {
    dash_ = dash;
}

void VehicleListener::receive(uint32_t id, uint8_t len, byte* data) {
    if (dash_ == nullptr) {
        ERROR_MSG("vehicle: dash not connected");
        return;
    }
    switch(id) {
        case 0x54A:
            receive54A(len, data);
            break;
        case 0x54B:
            receive54B(len, data);
            break;
        case 0x625:
            receive625(len, data);
            break;
    }
}

void VehicleListener::receive54A(uint8_t len, byte* data) {
    if (len != 8) {
        ERROR_MSG_VAL("vehicle: frame 0x54A has invalid length: 8 != ", len);
        return;
    }
    INFO_MSG_FRAME("vehicle: receive ", 0x54A, 8, data);
    dash_->setClimateDriverTemp(data[4]);
    dash_->setClimatePassengerTemp(data[5]);
}

void VehicleListener::receive54B(uint8_t len, byte* data) {
    if (len != 8) {
        ERROR_MSG_VAL("vehicle: frame 0x54B has invalid length: 8 != ", len);
        return;
    }
    INFO_MSG_FRAME("vehicle: receive ", 0x54B, 8, data);
    dash_->setClimateActive(!getBit(data, 0, 5));
    dash_->setClimateAuto(getBit(data, 0, 0));
    dash_->setClimateAc(getBit(data, 0, 3));
    dash_->setClimateDual(getBit(data, 3, 7));
    dash_->setClimateRecirculate(getBit(data, 3, 4));
    dash_->setClimateFanSpeed((data[2] + 1) / 2);

    switch(data[1]) {
        case 0x04:
            dash_->setClimateFace(true);
            dash_->setClimateFeet(false);
            dash_->setClimateFrontDefrost(false);
            break;
        case 0x08:
            dash_->setClimateFace(true);
            dash_->setClimateFeet(true);
            dash_->setClimateFrontDefrost(false);
            break;
        case 0x0C:
            dash_->setClimateFace(false);
            dash_->setClimateFeet(true);
            dash_->setClimateFrontDefrost(false);
            break;
        case 0x10:
            dash_->setClimateFace(false);
            dash_->setClimateFeet(true);
            dash_->setClimateFrontDefrost(true);
            break;
        case 0x34:
            dash_->setClimateFace(false);
            dash_->setClimateFeet(false);
            dash_->setClimateFrontDefrost(true);
            break;
        case 0x84:
            dash_->setClimateFace(true);
            dash_->setClimateFeet(false);
            dash_->setClimateFrontDefrost(false);
            break;
        case 0x88:
            dash_->setClimateFace(true);
            dash_->setClimateFeet(true);
            dash_->setClimateFrontDefrost(false);
            break;
        case 0x8C:
            dash_->setClimateFace(false);
            dash_->setClimateFeet(true);
            dash_->setClimateFrontDefrost(false);
            break;
        default:
            dash_->setClimateFace(false);
            dash_->setClimateFeet(false);
            dash_->setClimateFrontDefrost(false);
            break;
    }
}

void VehicleListener::receive625(uint8_t len, byte* data) {
    if (len != 6) {
        ERROR_MSG_VAL("vehicle: frame 0x625 has invalid length: 6 != ", len);
        return;
    }
    INFO_MSG_FRAME("vehicle: receive ", 0x625, 8, data);
    dash_->setClimateRearDefrost(getBit(data, 0, 0));
}
