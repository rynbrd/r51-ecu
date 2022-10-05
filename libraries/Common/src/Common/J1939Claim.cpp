#include "J1939Claim.h"

#include <Arduino.h>

namespace R51 {

size_t J1939Claim::printTo(Print& p) const {
    return p.print(address_, HEX) + p.print(":") + p.print(name_, HEX);
}

bool operator==(const J1939Claim& left, const J1939Claim& right) {
    return left.address() == right.address() && left.name() == right.name();
}

bool operator!=(const J1939Claim& left, const J1939Claim& right) {
    return left.address() != right.address() || left.name() != right.name();
}

}  // namespace R51
