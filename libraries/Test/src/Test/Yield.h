#ifndef _R51_TEST_YIELD_H_
#define _R51_TEST_YIELD_H_

#include <Arduino.h>
#include <Caster.h>
#include <Core.h>
#include <Foundation.h>

namespace R51 {

// A deep copy of a message.
class MessageCopy : public Printable {
    public:
        MessageCopy() : type_(Message::EMPTY) {}

        MessageCopy(const Message& msg) : type_(msg.type()) {
            switch (msg.type()) {
                case Message::EVENT:
                    event_ = *msg.event();
                    break;
                case Message::CAN_FRAME:
                    can_frame_ = *msg.can_frame();
                    break;
                case Message::J1939_CLAIM:
                    j1939_claim_ = *msg.j1939_claim();
                    break;
                case Message::J1939_MESSAGE:
                    j1939_message_ = *msg.j1939_message();
                    break;
                case Message::EMPTY:
                    break;
            }
        }

        enum Message::Type type() const { return type_; };

        const Event* event() const { return &event_; }

        const Canny::CAN20Frame* can_frame() const { return &can_frame_; }

        const J1939Claim* j1939_claim() const { return &j1939_claim_; }

        const Canny::J1939Message* j1939_message() const { return &j1939_message_; }

        size_t printTo(Print& p) const {
            switch (type_) {
                case Message::EMPTY:
                    return p.print("(EMPTY)");
                case Message::EVENT:
                    return p.print("(EVENT)") + event_.printTo(p);
                case Message::CAN_FRAME:
                    return p.print("(CAN_FRAME)") + can_frame_.printTo(p);
                case Message::J1939_CLAIM:
                    return p.print("(J1939_CLAIM)") +  j1939_claim_.printTo(p);
                case Message::J1939_MESSAGE:
                    return p.print("(J1939_MESSAGE)") + j1939_message_.printTo(p);
            }
            return 0;
        }

    private:
        Message::Type type_;
        Event event_;
        Canny::CAN20Frame can_frame_;
        J1939Claim j1939_claim_;
        Canny::J1939Message j1939_message_;
};

// A fake Yield implementation that collects copies of the yielded messages.
class FakeYield : public Caster::Yield<Message> {
    public:
        FakeYield() : messages_(new Array<MessageCopy>()) {}
        virtual ~FakeYield() { delete messages_; }

        // Return the number of messages stored in the object.
        size_t size() const { return messages_->size(); } 

        // Return a reference to the array of collected messages.
        const MessageCopy* messages() const { return messages_->data(); }

        // Clear the yielded messages from the object.
        void clear() { messages_->clear(); }

        // Collect a yielded message.
        void operator()(const Message& msg) const override { messages_->push_back(msg); }

    private:
        Array<MessageCopy>* messages_;
};

}  // namespace R51

#endif  // _R51_TEST_YIELD_H_
