#include "RealDash.h"

#include <Common.h>
#include "Debug.h"

using ::R51::Message;

void RealDashAdapter::handle(const Message& msg) {
    if (msg.type() != Message::EVENT) {
        return;
    }

    frame_.id(frame_id_);
    frame_.resize(8);
    frame_.data()[0] = msg.event().subsystem;
    frame_.data()[1] = msg.event().id;
    memcpy(frame_.data()+2, msg.event().data, 6);
    Canny::Error err = connection_->write(frame_);
    if (err != Canny::ERR_OK) {
        DEBUG_MSG_VAL("realdash: write error: ", err);
        DEBUG_MSG_VAL("realdash: discard frame: ", frame_);
    }
}

void RealDashAdapter::emit(const Caster::Yield<Message>& yield) {
    Canny::Error err = connection_->read(&frame_);
    if (err == Canny::ERR_FIFO) {
        return;
    } else if (err != Canny::ERR_OK) {
        DEBUG_MSG_VAL("realdash: read error: ", err);
        return;
    } else if (frame_.id() != frame_id_ || frame_.size() < 8) {
        return;
    }

    event_.subsystem = frame_.data()[0];
    event_.id = frame_.data()[1];
    memcpy(event_.data, frame_.data()+2, 6);
    yield(event_);
}
