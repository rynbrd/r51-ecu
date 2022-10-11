#include "J1939Gateway.h"

#include <Arduino.h>
#include <Canny.h>
#include "Message.h"

namespace R51 {
namespace {

using ::Canny::ERR_FIFO;
using ::Canny::ERR_OK;
using ::Canny::Error;
using ::Canny::J1939Message;
using ::Canny::NullAddress;
using ::Canny::j1939_name_arbitrary_address;
using ::Caster::Yield;

bool isAddressClaim(const J1939Message& msg) {
    return msg.pdu_format() == 0xEA;
}

bool isRequestAddressClaim(const J1939Message& msg, uint8_t address) {
    return msg.pdu_format() == 0xEA &&
        (msg.pdu_specific() == 0xFF || msg.pdu_specific() == address);
}

}  // namespace

void J1939Gateway::init(const Caster::Yield<Message>& yield) {
    if (name_ != 0) {
        writeClaim();
    }
    emitEvent(yield);
}

void J1939Gateway::handle(const Message& msg, const Yield<Message>&) {
    if (msg.type() == Message::J1939_MESSAGE &&
        (promiscuous_ || (msg.j1939_message().source_address() == address_ &&
                          address_ != NullAddress))) {
        write(msg.j1939_message());
    }
}

void J1939Gateway::handleAddressClaim(const J1939Message& msg,
        const Yield<Message>& yield) {
    if (address_ == NullAddress || address_ != msg.source_address()) {
        // no negotiation needed
        return;
    }

    if (msg.size() != 8) {
        // sanity check
        return;
    }

    if (name_ <= msg.name()) {
        // we have higher priority; emit our address claim
        writeClaim();
    } else if (j1939_name_arbitrary_address(name_)) {
        // we have lower priority and can negotiate a new address
        ++address_;
        if (address_ >= NullAddress) {
            address_ = 1;
        }
        if (address_ == preferred_address_) {
            // we've tried all of the addresses
            address_ = NullAddress;
        }
        writeClaim();
        emitEvent(yield);
    } else {
        // we have lower priority and cannot negotiate a new address
        address_ = NullAddress;
        writeClaim();
        emitEvent(yield);
    }
}

void J1939Gateway::emit(const Yield<Message>& yield) {
    // read a message off the J1939 bus
    Error err = can_->read(&msg_);
    if (err == ERR_FIFO)  {
        return;
    } else if (err != ERR_OK) {
        onReadError(err);
        return;
    }

    // handle address claim if configured
    if (name_ != 0) {
        if (isAddressClaim(msg_) && msg_.source_address() == address_) {
            handleAddressClaim(msg_, yield);
        } else if (isRequestAddressClaim(msg_, address_)) {
            writeClaim();
        }
    }

    // broadcast it on the internal bus
    if (promiscuous_ || msg_.dest_address() == address_ || msg_.broadcast()) {
        yield(msg_);
    }
}

void J1939Gateway::emitEvent(const Yield<Message>& yield) {
    J1939Claim claim(address_, name_);
    yield(claim);
}

void J1939Gateway::write(const J1939Message& msg) {
    Error err = can_->write(msg);
    if (err != ERR_OK && err != ERR_FIFO) {
        onWriteError(err, msg);
    }
}

void J1939Gateway::writeClaim() {
    J1939Message msg(0xEE00, address_, 0xFF,  0x06);
    msg.name(name_);
    write(msg);
}

}  // namespace R51
