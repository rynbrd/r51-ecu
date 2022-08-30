#include "CAN.h"

#include <Canny.h>
#include <Caster.h>

#include "Message.h"

namespace R51 {

using ::Canny::ERR_OK;
using ::Canny::ERR_FIFO;

void CANNode::handle(const Message& msg) {
    if (msg.type() != Message::CAN_FRAME || !writeFilter(msg.can_frame())) {
        return;
    }

    Canny::Error err;
    uint8_t attempts = 0;
    do {
        err = can_->write(msg.can_frame());
        attempts++;
    } while (err != ERR_OK && attempts <= retries_);

    if (err != ERR_OK) {
        onWriteError(err, msg.can_frame());
    }
}

void CANNode::emit(const Caster::Yield<Message>& yield) {
    Canny::Error err = can_->read(&frame_);
    if (err == ERR_OK) {
        if (readFilter(frame_)) {
            yield(Message(frame_));
        }
        return;
    }
    if (err != ERR_FIFO) {
        onReadError(err);
    }
}

}  // namespace R51
