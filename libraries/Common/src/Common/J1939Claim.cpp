#include "J1939Claim.h"

#include <Arduino.h>

namespace R51 {

bool operator==(const J1939Claim& left, const J1939Claim& right) {
    return left.address() == right.address() && left.name() == right.name();
}

bool operator!=(const J1939Claim& left, const J1939Claim& right) {
    return left.address() != right.address() || left.name() != right.name();
}

}  // namespace R51
