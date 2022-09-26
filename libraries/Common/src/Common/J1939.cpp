#include "J1939.h"

#include <Arduino.h>
#include <Canny.h>
#include "Message.h"

namespace R51 {
namespace {

using ::Canny::J1939Message;

enum class Emit : uint8_t {
    NONE = 0x00,
    EVENT = 0x01,
    J1939_CLAIM = 0x02,
    J1939_CANNOT_CLAIM = 0x03,
}; 

bool isAddressClaim(const J1939Message& msg) {
    return msg.pdu_format() == 0xEA;
}

bool isRequestAddressClaim(const J1939Message& msg, uint8_t address) {
    return msg.pdu_format() == 0xEA &&
        (msg.pdu_specific() == 0xFF || msg.pdu_specific() == address);
}

}  // namespace

void J1939AddressClaim::handle(const Message& msg, const Caster::Yield<Message>&) {
    //TODO: Yield messages directly.
    if (msg.type() != Message::J1939_MESSAGE ||
            (!msg.j1939_message().broadcast() &&
             msg.j1939_message().dest_address() != address_)) {
        return;
    }
    if (isAddressClaim(msg.j1939_message()) && msg.j1939_message().source_address() == address_) {
        handleAddressClaim(msg.j1939_message());
    } else if (isRequestAddressClaim(msg.j1939_message(), address_)) {
        emit_claim_ = true;
    }
}

void J1939AddressClaim::handleAddressClaim(const Canny::J1939Message& msg) {
    if (address_ == Canny::NullAddress || address_ != msg.source_address()) {
        // no negotiation needed
        return;
    }

    if (msg.size() != 8) {
        // sanity check
        return;
    }

    if (name_ <= msg.name()) {
        // we have higher priority; emit our address claim
        emit_claim_ = true;
    } else if (Canny::j1939_name_arbitrary_address(name_)) {
        // we have lower priority and can negotiate a new address
        ++address_;
        if (address_ >= Canny::NullAddress) {
            address_ = 1;
        }
        if (address_ == preferred_address_) {
            // we've tried all of the addresses
            address_ = Canny::NullAddress;
        }
        emit_event_ = true;
        emit_claim_ = true;
    } else {
        // we have lower priority and cannot negotiate a new address
        address_ = Canny::NullAddress;
        emit_event_ = true;
        emit_claim_ = true;
    }
}

void J1939AddressClaim::emit(const Caster::Yield<Message>& yield) {
    if (emit_claim_) {
        J1939Message msg(0xEE00, address_, 0xFF,  0x06);
        msg.name(name_);
        yield(msg);
        emit_claim_ = false;
    }
    if (emit_event_) {
        J1939Claim claim(address_, name_);
        yield(claim);
        emit_event_ = false;
    }
}

}  // namespace R51
