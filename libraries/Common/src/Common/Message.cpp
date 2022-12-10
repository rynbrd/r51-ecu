#include "Message.h"

namespace R51 {
namespace {

const void* makeRef(const Message& msg) {
    switch (msg.type()) {
        case Message::EMPTY:
            return nullptr;
        case Message::EVENT:
            return msg.event();
        case Message::CAN_FRAME:
            return msg.can_frame();
        case Message::J1939_CLAIM:
            return msg.j1939_claim();
        case Message::J1939_MESSAGE:
            return msg.j1939_message();
    }
    return nullptr;
}

template <typename T>
bool checkRef(const T* left, const T* right) {
    if (left == right) {
        return true;
    }
    if (left == nullptr || right == nullptr) {
        return false;
    }
    return *left == *right;
}

}  // namespace

bool operator==(const Message& left, const Message& right) {
    if (left.type() == right.type()) {
        switch (left.type()) {
            case Message::EVENT:
                return checkRef(left.event(), right.event());
            case Message::CAN_FRAME:
                return checkRef(left.can_frame(), right.can_frame());
            case Message::J1939_CLAIM:
                return checkRef(left.j1939_claim(), right.j1939_claim());
            case Message::J1939_MESSAGE:
                return checkRef(left.j1939_message(), right.j1939_message());
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
    switch (type()) {
        case EVENT:
            if (event() != nullptr) {
                return p.print(*event());
            }
            return 0;
        case CAN_FRAME:
            if (can_frame() != nullptr) {
                return p.print(*can_frame());
            }
            return 0;
        case J1939_CLAIM:
            if (j1939_claim() != nullptr) {
                return p.print(*j1939_claim());
            }
            return 0;
        case J1939_MESSAGE:
            if (j1939_message() != nullptr) {
                return p.print(*j1939_message());
            }
            return 0;
        case EMPTY:
            return 0;
    }
    return 0;
}

MessageValue::MessageValue(const Message& msg) : type_(msg.type()) {
    switch (type_) {
        case EMPTY:
            break;
        case EVENT:
            if (msg.event() == nullptr) {
                type_ = EMPTY;
            } else {
                event_ = *msg.event();
            }
            break;
        case CAN_FRAME:
            if (msg.can_frame() == nullptr) {
                type_ = EMPTY;
            } else {
                can_frame_ = *msg.can_frame();
            }
            break;
        case J1939_CLAIM:
            if (msg.j1939_claim() == nullptr) {
                type_ = EMPTY;
            } else {
                j1939_claim_ = *msg.j1939_claim();
            }
            break;
        case J1939_MESSAGE:
            if (msg.j1939_message() == nullptr) {
                type_ = EMPTY;
            } else {
                j1939_message_ = *msg.j1939_message();
            }
            break;
    }
}

MessageValue::MessageValue(const MessageValue& msg) : MessageValue((const Message&)msg) {}

MessageValue::~MessageValue() {
    switch (type_) {
        case EMPTY:
            break;
        case EVENT:
            event_.~Event();
            break;
        case CAN_FRAME:
            can_frame_.~Frame();
            break;
        case J1939_CLAIM:
            j1939_claim_.~J1939Claim();
            break;
        case J1939_MESSAGE:
            j1939_message_.~J1939Message();
            break;
    }
}

// Return the event referenced by the message. Return nullptr if type()
// != EVENT.
const Event* MessageValue::event() const {
    return type_ == EVENT ? &event_ : nullptr;
}

// Return the CAN frame referenced by the message. Return nullptr if
// type() != CAN_FRAME.
const Canny::CAN20Frame* MessageValue::can_frame() const {
    return type_ == CAN_FRAME ? &can_frame_ : nullptr;
}

// Return the J1939 address claim event referenced by the message.
// Return nullptr if type() != J1939_CLAIM.
const J1939Claim* MessageValue::j1939_claim() const {
    return type_ == J1939_CLAIM ? &j1939_claim_ : nullptr;
}

// Return the J1939 message referenced by the message. Return nullptr
// if type() != J1939_MESSAGE.
const Canny::J1939Message* MessageValue::j1939_message() const {
    return type_ == J1939_MESSAGE ? &j1939_message_ : nullptr;
}

MessageView::MessageView(const Message& msg) : type_(msg.type()), ref_(makeRef(msg)) {}

// Return the event referenced by the message. Return nullptr if type()
// != EVENT.
const Event* MessageView::event() const {
    return type_ == EVENT ? (Event*)ref_ : nullptr;
}

// Return the CAN frame referenced by the message. Return nullptr if
// type() != CAN_FRAME.
const Canny::CAN20Frame* MessageView::can_frame() const {
    return type_ == CAN_FRAME ? (Canny::CAN20Frame*)ref_ : nullptr;
}

// Return the J1939 address claim event referenced by the message.
// Return nullptr if type() != J1939_CLAIM.
const J1939Claim* MessageView::j1939_claim() const {
    return type_ == J1939_CLAIM ? (J1939Claim*)ref_ : nullptr;
}

// Return the J1939 message referenced by the message. Return nullptr
// if type() != J1939_MESSAGE.
const Canny::J1939Message* MessageView::j1939_message() const {
    return type_ == J1939_MESSAGE ? (Canny::J1939Message*)ref_ : nullptr;
}

}  // namespace R51
