#ifndef _R51_COMMON_MESSAGE_H_
#define _R51_COMMON_MESSAGE_H_

#include <Arduino.h>
#include <Canny.h>
#include "Event.h"
#include "J1939Claim.h"

namespace R51 {

class Message : public Printable {
    public:
        // The message type.
        enum Type {
            EMPTY,
            EVENT,
            CAN_FRAME,
            J1939_CLAIM,
            J1939_MESSAGE,
        };

        // Default constructor. Sets type to EMPTY.
        Message() : Message(EMPTY, nullptr) {}

        // Construct a message that references a system event.
        Message(Event* event) :
            type_(EVENT), ref_(event) {}

        // Construct a message that references a CAN frame.
        Message(Canny::Frame* can_frame) :
            type_(CAN_FRAME), ref_(can_frame) {}

        // Construct a message that references a J1939 address claim.
        Message(J1939Claim* j1939_claim) :
            type_(J1939_CLAIM), ref_(j1939_claim) {}

        // Construct a message that references a J1939 message.
        Message(Canny::J1939Message* j1939_message) :
            type_(J1939_MESSAGE), ref_(j1939_message) {}

        // Return the type of the mesasge.
        enum Type type() const { return type_; };

        // Return the event referenced by the message. Return nullptr if type()
        // != EVENT.
        const Event* event() const {
            return type_ == EVENT ? (Event*)ref_ : nullptr;
        }

        // Return the CAN frame referenced by the message. Return nullptr if
        // type() != CAN_FRAME.
        const Canny::Frame* can_frame() const {
            return type_ == CAN_FRAME ? (Canny::Frame*)ref_ : nullptr;
        }

        // Return the J1939 address claim event referenced by the message.
        // Return nullptr if type() != J1939_CLAIM.
        const J1939Claim* j1939_claim() const {
            return type_ == J1939_CLAIM ? (J1939Claim*)ref_ : nullptr;
        }

        // Return the J1939 message referenced by the message. Return nullptr
        // if type() != J1939_MESSAGE.
        const Canny::J1939Message* j1939_message() const {
            return type_ == J1939_MESSAGE ? (Canny::J1939Message*)ref_ : nullptr;
        }

        // Print the message. This prints the payload or nothing if empty.
        size_t printTo(Print& p) const;

    private:
        Type type_;
        void* ref_;

        Message(Type type, void* ref) : type_(type), ref_(ref) {}
};

// Return true if the two messages reference the same payload.
bool operator==(const Message& left, const Message& right);

// Return true if the two messages reference different payloads.
bool operator!=(const Message& left, const Message& right);

}  // namespace R51

#endif  // _R51_COMMON_MESSAGE_H_
