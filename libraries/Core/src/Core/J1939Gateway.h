#ifndef _R51_CORE_J1939_GATEWAY_H_
#define _R51_CORE_J1939_GATEWAY_H_

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include "J1939Claim.h"
#include "Message.h"

namespace R51 {

// Connects a bus to a J1939 network. The J1939 gateway can be configured to
// perform a J1939 address claim on init and respond appropriately to address
// claims and request for address claims from other nodes on the J1939 bus.
// These features may also be disabled when required. The gateway sends a
// J1939_CLAIM message on the bus any time the gateway assigns itself an
// address. This address will be Canny::NullAddress if the address claim fails.
class J1939Gateway : public Caster::Node<Message> {
    public:
        // Construct a gateway that communicates with the J1939 bus over the
        // given connection and claims the preferred address on init. The given
        // J1939 NAME is sentt with the address claim message. If NAME's
        // arbitrary address bit is 1 then the node will attempt to negotiate
        // for a new address if the preferred address is already claimed.
        // Normally the gateway discards incoming messages not addressed to
        // itself or the broadcast address. It also discards outgoing messages
        // whose source address does not match this gateway's address. The
        // gateway can be configired to disable this filtering by setting
        // promiscuous to true. A gateway which fails to claim an address will
        // filter all outgoing messages when promiscuous mode is disabled.
        J1939Gateway(Canny::Connection<Canny::J1939Message>* can,
                uint8_t preferred_address, uint64_t name, bool promiscuous) :
            can_(can), preferred_address_(preferred_address), address_(preferred_address),
            name_(name), promiscuous_(promiscuous) {}

        // Construct a gateway that communicates with the J1939 bus over the
        // given connection. The gateway does not participate in the address
        // claim process and does not respond to address claim or request
        // address claim messages. It instead hardcodes the given address.
        // Promiscuous mode can be enabled to avoid discarding messages
        // received on the bus. This is particularly useful for monitoring a
        // J1939 bus when there is no need to send messages. Generally you
        // would assign the null address in such cases.
        J1939Gateway(Canny::Connection<Canny::J1939Message>* can,
                uint8_t address, bool promiscuous) :
            can_(can), preferred_address_(address), address_(address), name_(0),
            promiscuous_(promiscuous) {}

        virtual ~J1939Gateway() = default;

        // Initialize the module. Assigns the preferred address and sends out
        // an initial address claim.
        void init(const Caster::Yield<Message>& yield) override;

        // Handle outgoing J1939 messages. Discards messsasges whose source
        // address is not address() when promiscuous mode is disabled.
        void handle(const Message& msg, const Caster::Yield<Message>& yield) override;

        // Read a message from the J1939 bus and broadcast it to the internal
        // bus. Messages whose destination address is neither address() or the
        // broadcast address are discarded when promiscuous mode is disabled.
        // Address claim messages are forwarded to the internal bus so that
        // attached nodes can identify specific endpoints on the bus. Emits a
        // J1939_CLAIM message any time the gateway claims a new address.
        void emit(const Caster::Yield<Message>&) override;

        // The current address of the gateway.
        uint8_t address() { return address_; }

        // The J1939 NAME of the gateway.
        uint64_t name() { return name_; }

        // Called when a J1939 message can't be read from the CAN bus.
        virtual void onReadError(Canny::Error) {}

        // Called when a J1939 message can't be written to the CAN bus.
        virtual void onWriteError(Canny::Error, const Canny::J1939Message&) {}
    private:
        void handleAddressClaim(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);

        void write(const Canny::J1939Message& msg);
        void writeClaim();
        void emitEvent(const Caster::Yield<Message>& yield);

        Canny::Connection<Canny::J1939Message>* can_;
        const uint8_t preferred_address_;
        uint8_t address_;
        uint64_t name_;
        bool promiscuous_;

        Canny::J1939Message msg_;
};

}  // namespace R51

#endif  // _R51_CORE_J1939_GATEWAY_H_
