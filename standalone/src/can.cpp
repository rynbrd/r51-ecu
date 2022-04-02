#include "can.h"

#include <CANBed.h>
#include "bus.h"
#include "debug.h"
#include "frame.h"

using CANBed::ERR_OK;

void CanNode::receive(const Broadcast& broadcast) {
    uint8_t err = can_->read(&frame_.id, nullptr, frame_.data, &frame_.len);
    if (err == CANBed::ERR_OK) {
        broadcast(frame_);
        return;
    }
    if (err != CANBed::ERR_FIFO) {
        ERROR_MSG("can: read failed");
    }
}

void CanNode::send(const Frame& frame) {
    uint8_t err;
    uint8_t attempts = 0;
    do {
        err = can_->write(frame.id, 0, (uint8_t*)frame.data, frame.len);
        attempts++;
    } while (err != ERR_OK && attempts <= retries_);

    if (err != ERR_OK) {
        ERROR_MSG_FRAME("can: dropped frame ", frame);
    }
}
