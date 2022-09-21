#include "Node.h"

#include <Canny.h>
#include <Caster.h>
#include <Common.h>
#include "Console.h"

namespace R51 {

void ConsoleNode::emit(const Caster::Yield<Message>& yield) {
    Reader::Error err;
    while (console_.stream()->available())  {
        if (command_->line()) {
            err = reader_.line();
        } else {
            err = reader_.word();
        }
        switch (err) {
            case Reader::EOW:
                command_ = command_->next(buffer_);
                break;
            case Reader::EOL:
                command_ = command_->next(buffer_);
                command_->run(&console_, buffer_, yield);
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

void ConsoleNode::handle(const Message& msg) {
    switch (msg.type()) {
        case Message::EVENT:
            console_.stream()->print("console: event recv ");
            console_.stream()->println(msg.event());
            break;
        case Message::CAN_FRAME:
            if (console_.can_filter()->match(msg.can_frame())) {
                console_.stream()->print("console: can recv ");
                console_.stream()->println(msg.can_frame());
            }
            break;
        case Message::J1939_MESSAGE:
            console_.stream()->print("console: j1939 recv ");
            console_.stream()->println(msg.j1939_message());
            break;
        case Message::EMPTY:
            break;
    }
}

}  // namespace R51
