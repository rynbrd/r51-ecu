#include "can.h"

#include <Canny.h>

#include "bus.h"
#include "debug.h"

using Canny::ERR_OK;

void CanNode::receive(const Broadcast& broadcast) {
    uint8_t err = can_->read(&frame_);
    if (err == Canny::ERR_OK) {
        broadcast(frame_);
        return;
    }
    if (err != Canny::ERR_FIFO) {
        ERROR_MSG("can: read failed");
    }
}

void CanNode::send(const Canny::Frame& frame) {
    uint8_t err;
    uint8_t attempts = 0;
    do {
        err = can_->write(frame);
        attempts++;
    } while (err != ERR_OK && attempts <= retries_);

    if (err != ERR_OK) {
        ERROR_MSG_VAL("can: dropped frame ", frame);
    }
}
