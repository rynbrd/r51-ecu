#ifndef _R51_COMMON_MESSAGE_H_
#define _R51_COMMON_MESSAGE_H_

#include <Arduino.h>
#include <Canny.h>
#include "Event.h"
#include "J1939Claim.h"

namespace R51 {

// Message type used on the internal bus.
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

        // Return the type of the message.
        virtual enum Type type() const { return EMPTY; };

        // Return the event referenced by the message. Return nullptr if type()
        // != EVENT.
        virtual const Event* event() const { return nullptr; }

        // Return the CAN frame referenced by the message. Return nullptr if
        // type() != CAN_FRAME.
        virtual const Canny::CAN20Frame* can_frame() const { return nullptr; }

        // Return the J1939 address claim event referenced by the message.
        // Return nullptr if type() != J1939_CLAIM.
        virtual const J1939Claim* j1939_claim() const { return nullptr; }

        // Return the J1939 message referenced by the message. Return nullptr
        // if type() != J1939_MESSAGE.
        virtual const Canny::J1939Message* j1939_message() const { return nullptr; }

        // Print the message. This prints the payload or nothing if empty.
        size_t printTo(Print& p) const;

    protected:
        Message() {}
        ~Message() = default;
};

// The message value type. Holds a copy of the message payload. This should be
// used where messages need to be buffered or stored for repeated use.
class MessageValue : public Message {
    public:
        // Default constructor. Sets type to EMPTY.
        MessageValue() : type_(EMPTY), empty_(0) {}
        ~MessageValue();

        // Copy another message into this one. If msg is a MessageView then its
        // referenced contents are copied into the new MessageValue.
        MessageValue(const Message& msg);
        MessageValue(const MessageValue& msg);

        // Construct a message holding a copy of a system event.
        MessageValue(const Event& event) :
            type_(EVENT), event_(event) {}

        // Construct a message holding a copy of a CAN frame.
        MessageValue(const Canny::CAN20Frame& can_frame) :
            type_(CAN_FRAME), can_frame_(can_frame) {}

        // Construct a message holding a copy of a J1939 address claim.
        MessageValue(const J1939Claim& j1939_claim) :
            type_(J1939_CLAIM), j1939_claim_(j1939_claim) {}

        // Construct a message holding a copy of a J1939 message.
        MessageValue(const Canny::J1939Message& j1939_message) :
            type_(J1939_MESSAGE), j1939_message_(j1939_message) {}

        // Return the type of the mesasge.
        enum Type type() const override { return type_; };

        // Return the event referenced by the message. Return nullptr if type()
        // != EVENT.
        const Event* event() const override;

        // Return the CAN frame referenced by the message. Return nullptr if
        // type() != CAN_FRAME.
        const Canny::CAN20Frame* can_frame() const override;

        // Return the J1939 address claim event referenced by the message.
        // Return nullptr if type() != J1939_CLAIM.
        const J1939Claim* j1939_claim() const override;

        // Return the J1939 message referenced by the message. Return nullptr
        // if type() != J1939_MESSAGE.
        const Canny::J1939Message* j1939_message() const;

        // Assignment operator.
        MessageValue& operator=(const Message& msg);
    private:
        Type type_;
        union {
            uint8_t empty_;
            Event event_;
            Canny::CAN20Frame can_frame_;
            J1939Claim j1939_claim_;
            Canny::J1939Message j1939_message_;
        };

        void copyFrom(const Message& msg);
};

// The message view type. Holds a reference to the message payload to avoid
// copying. Should be used to wrap a payload in all calls to yield().
class MessageView : public Message {
    public:
        // Default constructor. Sets type to EMPTY.
        MessageView() : type_(EMPTY), ref_(nullptr) {}

        // Copy a reference to another message into this one. If msg is a
        // MessageValue then the new MessageView references that MessageValue.
        MessageView(const Message& msg);

        // Construct a message that references a system event.
        MessageView(Event* event) :
            type_(EVENT), ref_(event) {}

        // Construct a message that references a CAN frame.
        MessageView(Canny::CAN20Frame* can_frame) :
            type_(can_frame == nullptr ? EMPTY : CAN_FRAME), ref_(can_frame) {}

        // Construct a message that references a J1939 address claim.
        MessageView(J1939Claim* j1939_claim) :
            type_(j1939_claim == nullptr ? EMPTY : J1939_CLAIM), ref_(j1939_claim) {}

        // Construct a message that references a J1939 message.
        MessageView(Canny::J1939Message* j1939_message) :
            type_(j1939_message == nullptr ? EMPTY : J1939_MESSAGE), ref_(j1939_message) {}

        // Return the type of the mesasge.
        enum Type type() const override { return type_; };

        // Return the event referenced by the message. Return nullptr if type()
        // != EVENT.
        const Event* event() const override;

        // Return the CAN frame referenced by the message. Return nullptr if
        // type() != CAN_FRAME.
        const Canny::CAN20Frame* can_frame() const override;

        // Return the J1939 address claim event referenced by the message.
        // Return nullptr if type() != J1939_CLAIM.
        const J1939Claim* j1939_claim() const override;

        // Return the J1939 message referenced by the message. Return nullptr
        // if type() != J1939_MESSAGE.
        const Canny::J1939Message* j1939_message() const override;

    private:
        Type type_;
        const void* ref_;
};

// Return true if the two messages reference the same payload.
bool operator==(const Message& left, const Message& right);

// Return true if the two messages reference different payloads.
bool operator!=(const Message& left, const Message& right);

}  // namespace R51

#endif  // _R51_COMMON_MESSAGE_H_
