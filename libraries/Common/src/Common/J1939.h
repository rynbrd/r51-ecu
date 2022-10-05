#ifndef _R51_COMMON_J1939_H_
#define _R51_COMMON_J1939_H_

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include "J1939Claim.h"
#include "Message.h"

namespace R51 {

class J1939Node : public Caster::Node<Message> {
    public:
        // Construct a new note that transmits J1939 messages over the given
        // connection.
        J1939Node(Canny::Connection* can) : can_(can) {}
        virtual ~J1939Node() = default;

        // Write a J1939 message to the CAN bus.
        void handle(const Message& msg, const Caster::Yield<Message>&) override;

        // Read a single J1939 Messages and broadcast it to the event bus.
        void emit(const Caster::Yield<Message>& yield) override;

        // Called when a J1939 message can't be read from the CAN bus.
        virtual void onReadError(Canny::Error) {}

        // Called when a J1939 message can't be written to the CAN bus.
        virtual void onWriteError(Canny::Error, const Canny::J1939Message&) {}

    private:
        Canny::Connection* can_;
        Canny::J1939Message msg_;
};

// Node to do address claim on J1939 networks.
class J1939AddressClaim : public Caster::Node<Message> {
    public:
        // Construct a node that claims the preferred address using the given
        // J1939 NAME. If NAME's arbitrary address bit is 1 then the node will
        // attempt to negotiate for a new address if the preferred address is
        // already claimed.
        J1939AddressClaim(uint8_t preferred_address, uint64_t name) :
            preferred_address_(preferred_address), address_(preferred_address),
            name_(name) {}

        // Initialize the module. Assigns the preferred address and sends out
        // an initial address claim.
        void init(const Caster::Yield<Message>& yield) override;

        // Consume address claim J1939 messages.
        void handle(const Message& msg, const Caster::Yield<Message>& yield) override;

        // Does nothing. 
        void emit(const Caster::Yield<Message>&) override {};

        // Address of this device.
        uint8_t address() { return address_; }

        // Name  of this device.
        uint64_t name() { return name_; }

    private:
        void handleAddressClaim(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);

        void emitEvent(const Caster::Yield<Message>& yield);
        void emitClaim(const Caster::Yield<Message>& yield);

        const uint8_t preferred_address_;
        uint8_t address_;
        uint64_t name_;
};

}  // namespace R51

#endif  // _R51_COMMON_J1939_H_
