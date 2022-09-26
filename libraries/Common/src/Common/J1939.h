#ifndef _R51_COMMON_J1939_H_
#define _R51_COMMON_J1939_H_

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include "J1939Claim.h"
#include "Message.h"

namespace R51 {

// Node to do address claim on J1939 networks.
class J1939AddressClaim : public Caster::Node<Message> {
    public:
        // Construct a node that claims the preferred address using the given
        // J1939 NAME. If NAME's arbitrary address bit is 1 then the node will
        // attempt to negotiate for a new address if the preferred address is
        // already claimed.
        J1939AddressClaim(uint8_t preferred_address, uint64_t name) :
            preferred_address_(preferred_address), address_(preferred_address),
            name_(name), emit_event_(true), emit_claim_(true) {}

        // Consume address claim J1939 messages.
        void handle(const Message& msg, const Caster::Yield<Message>&) override;

        // Emit address claim J1939 messages.
        void emit(const Caster::Yield<Message>& yield) override;

        // Address of this device.
        uint8_t address() { return address_; }

        // Name  of this device.
        uint64_t name() { return name_; }

    private:
        void handleAddressClaim(const Canny::J1939Message& msg);
        uint8_t preferred_address_;
        uint8_t address_;
        uint64_t name_;
        bool emit_event_;
        bool emit_claim_;
};

}  // namespace R51

#endif  // _R51_COMMON_J1939_H_
