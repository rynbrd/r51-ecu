#include "Message.h"

namespace R51 {

bool operator==(const Message& left, const Message& right) {
    return left.type_ == right.type_ &&
        left.ref_ == right.ref_;
}

bool operator!=(const Message& left, const Message& right) {
    return left.type_ != right.type_ ||
        left.ref_ != right.ref_;
}

size_t Message::printTo(Print& p) const {
    switch (type_) {
        case EVENT:
            return p.print(event());
        case CAN_FRAME:
            return p.print(can_frame());
        case EMPTY:
            break;
    }
    return 0;
}

}  // namespace R51
