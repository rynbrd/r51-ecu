#include "Event.h"

namespace R51 {
namespace {

size_t printHexByte(Print& p, uint8_t b) {
    if (b <= 0x0F) {
        p.print("0");
    }
    p.print(b, HEX);
    return 2;
}

}

Event::Event() : subsystem(0), id(0), size(0) {
    memset(data, 0xFF, 6);
}

Event::Event(uint8_t subsystem, uint8_t id) :
        subsystem(subsystem), id(id), size(0) {
    memset(data, 0xFF, 6);
}

size_t Event::printTo(Print& p) const {
    size_t n = 0;
    n += printHexByte(p, subsystem);
    n += p.print(":");
    n += printHexByte(p, id);
    for (uint8_t i = 0; i < size; i++) {
        n += p.print(":");
        n += printHexByte(p, data[i]);
    }
    return n;
}

bool operator==(const Event& left, const Event& right) {
    return left.subsystem == right.subsystem && left.id == right.id &&
        left.size == right.size && memcmp(left.data, right.data, left.size) == 0;
}

// Return true if the two system events are not equal
bool operator!=(const Event& left, const Event& right) {
    return left.subsystem != right.subsystem || left.id != right.id ||
        left.size != right.size || memcmp(left.data, right.data, left.size) != 0;
}

}  // namespace R51
