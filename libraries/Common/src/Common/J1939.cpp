#include "J1939.h"

#include <Arduino.h>
#include <Canny.h>
#include "Message.h"

namespace R51 {
namespace {

using ::Canny::ERR_FIFO;
using ::Canny::ERR_OK;
using ::Canny::Error;
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

void J1939Gateway::handle(const Message& msg, const Caster::Yield<Message>&) {
    if (msg.type() != Message::J1939_MESSAGE) {
        return;
    }
    Error err = can_->write(msg.j1939_message());
    if (err != ERR_OK && err != ERR_FIFO) {
        onWriteError(err, msg.j1939_message());
    }
}

void J1939Gateway::emit(const Caster::Yield<Message>& yield) {
    Error err = can_->read(&msg_);
    if (err == ERR_OK) {
        yield(msg_);
    } else if (err != ERR_FIFO) {
        onReadError(err);
    }
}

void J1939AddressClaim::init(const Caster::Yield<Message>& yield) {
    emitClaim(yield);
    emitEvent(yield);
}

void J1939AddressClaim::handle(const Message& msg, const Caster::Yield<Message>& yield) {
    if (msg.type() != Message::J1939_MESSAGE ||
            (!msg.j1939_message().broadcast() &&
             msg.j1939_message().dest_address() != address_)) {
        return;
    }
    if (isAddressClaim(msg.j1939_message()) && msg.j1939_message().source_address() == address_) {
        handleAddressClaim(msg.j1939_message(), yield);
    } else if (isRequestAddressClaim(msg.j1939_message(), address_)) {
        emitClaim(yield);
    }
}

void J1939AddressClaim::handleAddressClaim(const Canny::J1939Message& msg,
        const Caster::Yield<Message>& yield) {
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
        emitClaim(yield);
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
        emitClaim(yield);
        emitEvent(yield);
    } else {
        // we have lower priority and cannot negotiate a new address
        address_ = Canny::NullAddress;
        emitClaim(yield);
        emitEvent(yield);
    }
}

void J1939AddressClaim::emitEvent(const Caster::Yield<Message>& yield) {
        J1939Claim claim(address_, name_);
        yield(claim);
}

void J1939AddressClaim::emitClaim(const Caster::Yield<Message>& yield) {
        J1939Message msg(0xEE00, address_, 0xFF,  0x06);
        msg.name(name_);
        yield(msg);
}

}  // namespace R51
