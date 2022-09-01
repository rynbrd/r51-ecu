#include "Console.h"

#include <Canny.h>
#include <Caster.h>
#include <Common.h>

namespace R51 {

void Console::emit(const Caster::Yield<Message>& yield) {
    while (stream_->available())  {
        switch (reader_.word()) {
            case Reader::EOW:
                command_ = command_->next(buffer_);
                break;
            case Reader::EOL:
                command_ = command_->next(buffer_);
                command_->run(stream_, yield);
                command_ = &root_;
                break;
            case Reader::OVERRUN:
                command_ = &root_;
                break;
            default:
                break;
        }
    }
}

void Console::handle(const Message& msg) {
    if (msg.type() == Message::EMPTY || !writeFilter(msg)) {
        return;
    }
    switch (msg.type()) {
        case Message::EVENT:
            stream_->print("console: recv ");
            stream_->println(msg.event());
            break;
        case Message::CAN_FRAME:
            stream_->print("console: recv ");
            stream_->println(msg.can_frame());
            break;
        case Message::EMPTY:
            break;
    }
}

}  // namespace R51
