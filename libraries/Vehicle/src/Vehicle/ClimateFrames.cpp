#include "ClimateFrames.h"

#include <Arduino.h>
#include <Canny.h>
#include <Common.h>

namespace R51 {
namespace {

bool isInit(const Canny::Frame* frame) {
    return frame->data()[0] == 0x80;
}

}  // namespace

ClimateSystemControlFrame::ClimateSystemControlFrame(bool ready) : Canny::Frame(0x540, 0, 8) {
    data()[0] = 0x80;
    if (ready) {
        this->ready();
    }
}

void ClimateSystemControlFrame::ready() {
    if (!isInit(this)) {
        return;
    }
    data()[0] = 0x60;
    data()[1] = 0x40;
    data()[6] = 0x04;
}

void ClimateSystemControlFrame::turnOff() {
    if (isInit(this)) {
        return;
    }
    toggleBit(data(), 6, 7);
}

void ClimateSystemControlFrame::toggleAuto() {
    if (isInit(this)) {
        return;
    }
    toggleBit(data(), 6, 5);
}

void ClimateSystemControlFrame::toggleAC() {
    if (isInit(this)) {
        return;
    }
    toggleBit(data(), 5, 3);
}

void ClimateSystemControlFrame::toggleDual() {
    if (isInit(this)) {
        return;
    }
    toggleBit(data(), 6, 3);
}

void ClimateSystemControlFrame::cycleMode() {
    if (isInit(this)) {
        return;
    }
    toggleBit(data(), 6, 0);
}

void ClimateSystemControlFrame::toggleDefrost() {
    if (isInit(this)) {
        return;
    }
    toggleBit(data(), 6, 1);
}

void ClimateSystemControlFrame::incDriverTemp() {
    if (isInit(this)) {
        return;
    }
    toggleBit(data(), 5, 5);
    data()[3]++;
}

void ClimateSystemControlFrame::decDriverTemp() {
    if (isInit(this)) {
        return;
    }
    toggleBit(data(), 5, 5);
    data()[3]--;
}

void ClimateSystemControlFrame::incPassengerTemp() {
    if (isInit(this)) {
        return;
    }
    toggleBit(data(), 5, 5);
    data()[4]++;
}

void ClimateSystemControlFrame::decPassengerTemp() {
    if (isInit(this)) {
        return;
    }
    toggleBit(data(), 5, 5);
    data()[4]--;
}

ClimateFanControlFrame::ClimateFanControlFrame(bool ready) : Canny::Frame(0x541, 0, 8) {
    data()[0] = 0x80;
    if (ready) {
        this->ready();
    }
}

void ClimateFanControlFrame::ready() {
    if (!isInit(this)) {
        return;
    }
    data()[0] = 0x00;
}

void ClimateFanControlFrame::toggleRecirculate() {
    if (isInit(this)) {
        return;
    }
    toggleBit(data(), 1, 6);
}

void ClimateFanControlFrame::incFanSpeed() {
    if (isInit(this)) {
        return;
    }
    toggleBit(data(), 0, 5);
}

void ClimateFanControlFrame::decFanSpeed() {
    if (isInit(this)) {
        return;
    }
    toggleBit(data(), 0, 4);
}

}  // namespace R51
