#include "RealDash.h"

#include <Common.h>
#include "Debug.h"

using ::R51::Message;

namespace R51 {

void RealDashAdapter::handle(const Message& msg, const Caster::Yield<Message>&) {
    if (msg.type() != Message::EVENT) {
        return;
    }

    frame_.id(frame_id_);
    frame_.resize(8);
    frame_.data()[0] = msg.event()->subsystem;
    frame_.data()[1] = msg.event()->id;
    memcpy(frame_.data()+2, msg.event()->data, 6);
    Canny::Error err = connection_->write(frame_);
    if (err != Canny::ERR_OK) {
        DEBUG_MSG_VAL("realdash: write error: ", err);
        DEBUG_MSG_VAL("realdash: discard frame: ", frame_);
    }
}

void RealDashAdapter::emit(const Caster::Yield<Message>& yield) {
    if (hb_id_ > 0 && hb_ticker_.active()) {
        frame_.id(hb_id_);
        frame_.resize(8);
        frame_.data()[0] = hb_counter_++;
        memset(frame_.data()+1, 0, 7);
        hb_ticker_.reset();

        Canny::Error err = connection_->write(frame_);
        if (err != Canny::ERR_OK) {
            DEBUG_MSG_VAL("realdash: write error: ", err);
            DEBUG_MSG_VAL("realdash: discard frame: ", frame_);
        }
    }

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
    yield(MessageView(&event_));
}

}  // namespace R51
