#include "Node.h"

#include <Canny.h>
#include <Caster.h>
#include <Core.h>
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

void ConsoleNode::handle(const Message& msg, const Caster::Yield<R51::Message>&) {
    switch (msg.type()) {
        case Message::EVENT:
            if (!console_.event_mute()) {
                console_.stream()->print("console: event recv ");
                msg.event()->printTo(*console_.stream());
                console_.stream()->println();
            }
            break;
        case Message::CAN_FRAME:
            if (console_.can_filter()->match(*msg.can_frame())) {
                console_.stream()->print("console: can recv ");
                msg.can_frame()->printTo(*console_.stream());
                console_.stream()->println();
            }
            break;
        case Message::J1939_CLAIM:
            if (!console_.j1939_mute()) {
                console_.stream()->print("console: j1939 claim ");
                if (msg.j1939_claim()->address() <= 0x0F) {
                    console_.stream()->print("0x0");
                } else {
                    console_.stream()->print("0x");
                }
                console_.stream()->println(msg.j1939_claim()->address(), HEX);
            }
            break;
        case Message::J1939_MESSAGE:
            if (!console_.j1939_mute()) {
                console_.stream()->print("console: j1939 recv ");
                msg.j1939_message()->printTo(*console_.stream());
                console_.stream()->println();
            }
            break;
        case Message::EMPTY:
            break;
    }
}

}  // namespace R51
