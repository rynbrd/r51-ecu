#include "J1939Claim.h"

#include <Arduino.h>
#include <ByteOrder.h>

namespace R51 {
namespace {

void printHexByte(Print& p, uint8_t byte) {
    if (byte <= 0x0F)  {
        p.print('0');
    }
    p.print(byte, HEX);
}

size_t printHexUint64(Print& p, uint64_t value) {
    uint8_t* bytes = (uint8_t*)&value;
    if (ByteOrder::host() == ByteOrder::BIG) {
        for (uint8_t i = 0; i < 8; ++i) {
            printHexByte(p, bytes[i]);
        }
    } else {
        for (uint8_t i = 8; i > 0; --i) {
            printHexByte(p, bytes[i - 1]);
        }
    }
    return 16;
}

}  // namespace

size_t J1939Claim::printTo(Print& p) const {
    return p.print(address_, HEX) + p.print(":") + printHexUint64(p, name_);
}

bool operator==(const J1939Claim& left, const J1939Claim& right) {
    return left.address() == right.address() && left.name() == right.name();
}

bool operator!=(const J1939Claim& left, const J1939Claim& right) {
    return left.address() != right.address() || left.name() != right.name();
}

}  // namespace R51
