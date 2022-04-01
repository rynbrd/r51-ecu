#include "can.h"

#include "debug.h"
#include "CANBed.h"


void CanNode::begin() {
    if (CAN.begin(bitrate_) != CANBed::ERR_OK) {
        ERROR_MSG("can: init failed");
        while (true) { delay(1000); }
    }
}

void CanNode::receive(const Broadcast& broadcast) {
    if (!init_) {
        return;
    }

    uint8_t err = CAN.read(&frame_.id, nullptr, frame_.data, &frame_.len);
    if (err == CANBed::ERR_OK) {
        broadcast(frame_);
        return;
    }
    if (err != CANBed::ERR_FIFO) {
        ERROR_MSG("can: read failed");
    }
}

void CanNode::send(const Frame& frame) {
    if (!init_) {
        return;
    }

    uint8_t err;
    uint8_t attempts = 0;
    do {
        err = CAN.write(frame.id, 0, (uint8_t*)frame.data, frame.len);
        attempts++;
    } while (err != CANBed::ERR_OK && attempts <= retries_);

    if (err != CANBed::ERR_OK) {
        ERROR_MSG_FRAME("can: dropped frame ", frame);
    }
}
