#ifndef _R51_TEST_YIELD_H_
#define _R51_TEST_YIELD_H_

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>
#include "Array.h"

namespace R51 {

// A deep copy of a message.
class MessageCopy : public Printable {
    public:
        MessageCopy() : type_(Message::EMPTY) {}

        MessageCopy(const Message& msg) : type_(msg.type()) {
            switch (msg.type()) {
                case Message::EVENT:
                    event_ = msg.event();
                    break;
                case Message::CAN_FRAME:
                    can_frame_ = msg.can_frame();
                    break;
                case Message::J1939_MESSAGE:
                    j1939_message_ = msg.j1939_message();
                    break;
                case Message::EMPTY:
                    break;
            }
        }

        enum Message::Type type() const { return type_; };

        const Event& event() const { return event_; }

        const Canny::Frame& can_frame() const { return can_frame_; }

        const Canny::J1939Message& j1939_messaage() const { return j1939_message_; }

        size_t printTo(Print& p) const {
            switch (type_) {
                case Message::EMPTY:
                    return p.print("(EMPTY)");
                case Message::EVENT:
                    return p.print("(EVENT)") + p.print(event_);
                case Message::CAN_FRAME:
                    return p.print("(CAN_FRAME)") + p.print(can_frame_);
                case Message::J1939_MESSAGE:
                    return p.print("(J1939_MESSAGE)") + p.print(j1939_message_);
            }
        }

    private:
        Message::Type type_;
        Event event_;
        Canny::Frame can_frame_;
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
        void operator()(const Message& msg) const override { messages_->push(msg); }

    private:
        Array<MessageCopy>* messages_;
};

}  // namespace R51

#endif  // _R51_TEST_YIELD_H_
