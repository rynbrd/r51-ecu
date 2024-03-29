#ifndef _R51_CORE_J1939_ADAPTER_H_
#define _R51_CORE_J1939_ADAPTER_H_

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include "Event.h"
#include "Message.h"

namespace R51 {

// Translates Events to and from J1939 messages. Only J1939 messages directed
// at the controller are translated to events. The node does not operate until
// a source address has been claimed. Incoming messages must be of PGN 0xFF00.
// Outgoing messages have a PGN of 0xEF00.
class J1939Adapter : public Caster::Node<Message> {
    public:
        J1939Adapter();
        virtual ~J1939Adapter() = default;

        // Translate Events to and from J1939 messages.
        void handle(const Message& msg, const Caster::Yield<Message>& yield) override;

        // Implemented by child classes to filter and route events to specific
        // J1939 destinations. Should return the J1939 address of the
        // destination to route the event to, either a specific address or 0xFF
        // for broadcast. If the null address is returned then the event is
        // dropped. Broadcast events are sent with PGN 0xFF00. Addressed events
        // are sent with PGN 0xEF00.
        virtual uint8_t route(const Event&) { return 0xFF; }

        // Implemented by child classes to filter events read from the J1939
        // bus. Return false if an event read from the J1939 bus should be
        // broadcast to the internal bus.
        virtual bool readFilter(const Event&) { return true; }
    private:
        void handleEvent(const Event& event,
                const Caster::Yield<Message>& yield);
        void handleJ1939Message(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);

        Canny::J1939Message j1939_;
        Event event_;
};

}  // namespace R51

#endif  // _R51_CORE_J1939_ADAPTER_H_
