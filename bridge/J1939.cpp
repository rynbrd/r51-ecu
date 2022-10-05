#include "J1939.h"

#include <Common.h>

using ::R51::Message;

namespace R51 {

J1939Adapter::J1939Adapter(Canny::Connection* connection, uint8_t address) :
        connection_(connection), address_(address), j1939_(0xFF00, address) {
    j1939_.reserve(8);
}

void J1939Adapter::handle(const Message& msg, const Caster::Yield<Message>&) {
    if (msg.type() != Message::EVENT) {
        return;
    }
    j1939_.resize(8);
    j1939_.data()[0] = msg.event().subsystem;
    j1939_.data()[1] = msg.event().id;
    memcpy(j1939_.data()+2, msg.event().data, 6);
    connection_->write(j1939_);
    // errors are logged by the connection
}

void J1939Adapter::emit(const Caster::Yield<Message>& yield) {
    Canny::Error err = connection_->read(&j1939_);
    if (err != Canny::ERR_OK) {
        // errors are logged by the connection
        return;
    }
    if (j1939_.size() < 8 || j1939_.broadcast() || j1939_.pgn() != 0xEF00) {
        // ignore messages that don't have events in them
        return;
    }

    event_.subsystem = j1939_.data()[0];
    event_.id = j1939_.data()[1];
    memcpy(event_.data, j1939_.data()+2, 6);
    yield(event_);
}

}  // namespace R51
