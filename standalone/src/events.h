#ifndef __R51_EVENTS_H__
#define __R51_EVENTS_H__

#include <Arduino.h>
#include <Canny.h>

struct SystemEvent {
    // Unique Event identifier. This is unique among all events.
    uint8_t id;
    // Event data. Empty bytes are padded with 1s.
    uint8_t data[5];
};

class Message {
    public:
        // The message type.
        enum Type {
            SYSTEM_EVENT,
            CAN_FRAME,
        };

        // Construct a message that references a system event.
        Message(const SystemEvent& system_event) :
            type_(SYSTEM_EVENT), ref_(&system_event) {}

        // Construct a message that references a CAN frame.
        Message(const Canny::Frame& can_frame) :
            type_(CAN_FRAME), ref_(&can_frame) {}

        // Return the type of the mesasge.
        enum Type type() const { return type_; };

        // Return the system event referenced by the message. Requires that
        // type() returns SYSTEM_EVENT or undefined behavior will result.
        const SystemEvent& system_event() const { return *((SystemEvent*)ref_); }

        // Return the can frame referenced by the message. Requires that type()
        // returns CAN_FRAME or undefined behavior will result.
        const Canny::Frame& can_frame() const { return *((Canny::Frame*)ref_); }

    private:
        const Type type_;
        const void* ref_;
};

#endif  // __R51_EVENTS_H__
