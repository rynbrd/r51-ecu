#include "CAN.h"

namespace R51 {

using ::Canny::ERR_FIFO;
using ::Canny::ERR_OK;
using ::Canny::Error;
using ::Canny::Frame;

void CANNode::handle(const Message& msg, const Caster::Yield<Message>&) {
    if (msg.type() != Message::CAN_FRAME) {
        return;
    }
    Error err = can_->write(msg.can_frame());
    if (err != ERR_OK && err != ERR_FIFO) {
        onWriteError(err, msg.can_frame());
    }
}

void CANNode::emit(const Caster::Yield<Message>& yield) {
    Error err = can_->read(&frame_);
    if (err == ERR_OK) {
        yield(frame_);
    } else if (err != ERR_FIFO) {
        onReadError(err);
    }
}

}  // namespace R51
