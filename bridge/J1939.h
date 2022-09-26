#ifndef _R51_BRIDGE_J1939_H_
#define _R51_BRIDGE_J1939_H_

#include "Config.h"

#include <Canny.h>
#include <Caster.h>
#include <Common.h>

namespace R51 {

class J1939Adapter : public Caster::Node<Message> {
    public:
        J1939Adapter(Canny::Connection* connection, uint8_t address);

        // Encode and send an Event message over the J1939 bus.
        void handle(const Message& msg, const Caster::Yield<Message>&) override;

        // Yield received Events from the J1939 bus.
        void emit(const Caster::Yield<Message>& yield) override;
    private:
        Canny::Connection* connection_;
        uint8_t address_;
        Canny::J1939Message j1939_;
        Event event_;
};

}  // namespace R51

#endif  // _R51_BRIDGE_J1939_H_
