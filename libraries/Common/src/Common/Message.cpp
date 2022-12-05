#include "Message.h"

namespace R51 {

bool operator==(const Message& left, const Message& right) {
    if (left.type() == right.type()) {
        switch (left.type()) {
            case Message::EVENT:
                return *left.event() == *right.event();
            case Message::CAN_FRAME:
                return *left.can_frame() == *right.can_frame();
            case Message::J1939_CLAIM:
                return *left.j1939_claim() == *right.j1939_claim();
            case Message::J1939_MESSAGE:
                return *left.j1939_message() == *right.j1939_message();
            case Message::EMPTY:
                return true;
        }
    }
    return false;
}

bool operator!=(const Message& left, const Message& right) {
    return !(left == right);
}

size_t Message::printTo(Print& p) const {
    switch (type_) {
        case EVENT:
            return p.print(*event());
        case CAN_FRAME:
            return p.print(*can_frame());
        case J1939_CLAIM:
            return p.print(*j1939_claim());
        case J1939_MESSAGE:
            return p.print(*j1939_message());
        case EMPTY:
            break;
    }
    return 0;
}

}  // namespace R51
