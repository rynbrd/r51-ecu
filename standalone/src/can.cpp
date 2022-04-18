#include "can.h"

#include <Canny.h>
#include <Caster.h>

#include "debug.h"
#include "events.h"

using Canny::ERR_OK;

void CanNode::handle(const Message& msg) {
    if (msg.type() != Message::CAN_FRAME || !writeFilter(msg.can_frame())) {
        return;
    }

    uint8_t err;
    uint8_t attempts = 0;
    do {
        err = can_->write(msg.can_frame());
        attempts++;
    } while (err != ERR_OK && attempts <= retries_);

    if (err != ERR_OK) {
        ERROR_MSG_VAL("can: dropped frame ", msg.can_frame());
    }
}

void CanNode::emit(const Caster::Yield<Message>& yield) {
    uint8_t err = can_->read(&frame_);
    if (err == Canny::ERR_OK) {
        if (readFilter(frame_)) {
            yield(frame_);
        }
        return;
    }
    if (err != Canny::ERR_FIFO) {
        ERROR_MSG("can: read failed");
    }
}
