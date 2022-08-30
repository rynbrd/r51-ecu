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

}  // namespace R51
