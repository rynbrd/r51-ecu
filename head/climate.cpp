#include "climate.h"

#include <Arduino.h>
#include "can.h"
#include "debug.h"

namespace ECU {

ClimateControl::ClimateControl() {
    running_ = false;
    emit_ = true;
    memset(frame540_.data, 0, 8);
    memset(frame541_.data, 0, 8);
    frame540_.data[0] = 0x80;
    frame541_.data[1] = 0x80;
}

bool ClimateControl::isOff() const {
    return off_;
}

void ClimateControl::turnOff() {
    frame540_.data[6] = frame540_.data[6] ^ (1<<7);
    emit_ = true;
}

bool ClimateControl::isAuto() const {
    return auto_;
}

void ClimateControl::toggleAuto() {
    frame540_.data[6] = frame540_.data[6] ^ (1<<5);
    emit_ = true;
}

bool ClimateControl::isAc() const {
    return ac_;
}

void ClimateControl::toggleAc() {
    frame540_.data[5] = frame540_.data[5] ^ (1<<3);
    emit_ = true;
}

bool ClimateControl::isFace() const {
    return face_;
}

bool ClimateControl::isFeet() const {
    return feet_;
}

bool ClimateControl::isWindshield() const {
    return windshield_;
}

void ClimateControl::cycleMode() {
    frame540_.data[6] = frame540_.data[6] ^ 1;
    emit_ = true;
}

void ClimateControl::toggleFrontDefrost() {
    frame540_.data[6] = frame540_.data[6] ^ (1<<1);
    emit_ = true;
}

bool ClimateControl::isDualZone() const {
    return dual_;
}

void ClimateControl::toggleDualZone() {
    frame540_.data[6] = frame540_.data[6] ^ (1<<3);
    emit_ = true;
}

bool ClimateControl::isRecirculating() const {
    return recirculate_;
}

void ClimateControl::toggleRecirculation() {
    frame541_.data[1] = frame541_.data[1] ^ (1<<6);
    emit_ = true;
}

uint8_t ClimateControl::getFanSpeed() const {
    return fan_speed_;
}

void ClimateControl::increaseFanSpeed() {
    frame541_.data[0] = frame541_.data[0] ^ (1<<5);
    emit_ = true;
}

void ClimateControl::decreaseFanSpeed() {
    frame541_.data[0] = frame541_.data[0] ^ (1<<4);
    emit_ = true;
}

uint8_t ClimateControl::getDriverTemp() const {
    return driver_temp_;
}

void ClimateControl::setDriverTemp(uint8_t temp) {
    if (!operational() || driver_temp_ == 0) {
        return;
    }
    frame540_.data[5] = frame540_.data[5] ^ (1<<5);
    frame540_.data[3] = driver_temp_ + 0xB8;
    emit_ = true;
}

uint8_t ClimateControl::getPassengerTemp() const {
    return passenger_temp_;
}

void ClimateControl::setPassengerTemp(uint8_t temp) {
    if (!operational() || passenger_temp_ == 0) {
        return;
    }
    frame540_.data[5] = frame540_.data[5] ^ (1<<5);
    frame540_.data[4] = passenger_temp_ - 0x33;
    emit_ = true;
}

void ClimateControl::process(const Frame& frame) {
    switch(frame.id) {
        case 0x54A:
            process54A(frame);
            break;
        case 0x54B:
            process54B(frame);
            break;
    }
}

void ClimateControl::process54A(const Frame& frame) {
    driver_temp_ = frame.data[4];
    passenger_temp_ = frame.data[5];
}

void ClimateControl::process54B(const Frame& frame) {
    off_ = frame.data[0] & (1<<5) > 0;
    running_ = (frame.data[0] & (1<<7)) > 0 && off_;
    auto_ = frame.data[0] & 1 > 0;
    ac_ = frame.data[0] & (1<<3) > 0;
    fan_speed_ = (frame.data[2]+1) / 2;
    dual_ = frame.data[3] & (1<<7) > 0;
    recirculate_ = frame.data[3] & (1<<4) > 0;

    switch (frame.data[1]) {
        case 0x04:
            face_ = true;
            feet_ = false;
            windshield_ = false;
            break;
        case 0x08:
            face_ = true;
            feet_ = true;
            windshield_ = false;
            break;
        case 0x0C:
            face_ = false;
            feet_ = true;
            windshield_ = false;
            break;
        case 0x10:
            face_ = false;
            feet_ = true;
            windshield_ = true;
            break;
        case 0x34:
            face_ = false;
            feet_ = false;
            windshield_ = true;
            break;
        case 0x84:
            face_ = true;
            feet_ = false;
            windshield_ = false;
            break;
        case 0x88:
            face_ = true;
            feet_ = true;
            windshield_ = false;
            break;
        case 0x8C:
            face_ = false;
            feet_ = true;
            windshield_ = false;
            break;
        default:
            face_ = false;
            feet_ = false;
            windshield_ = false;
            break;
    }

    if (running_ && frame540_.data[0] & (1<<7) > 0) {
        frame540_.data[0] = 0x00;
        frame541_.data[0] = 0x00;
    }
}

void ClimateControl::emit(CanTranceiver* tranceiver) {
    if (millis() - heartbeat_ > 200) {
        emit_ = true;
    }

    if (emit_) {
        if (tranceiver->write(frame540_) != OK) {
            Debug.println("error sending climate frame 540");
        }
        if (tranceiver->write(frame541_) != OK) {
            Debug.println("error sending climate frame 541");
        }

        if (running_ && frame540_.data[0] == 0x00) {
            frame540_.data[0] = 0x60;
            frame540_.data[1] = 0x40;
            frame540_.data[6] = 0x04;
        }

        emit_ = false;
        heartbeat_ = millis();
    }
}

}
