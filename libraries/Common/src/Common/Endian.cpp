#include "Endian.h"

#include <Arduino.h>

namespace R51 {

bool little_endian() {
    union {
        uint16_t a;
        uint8_t b[2];
    } endian = {0x0001};
    return endian.b[0] == 0x01;
}

void NetworkToUInt32(uint32_t* dest, const uint8_t* src) {
    if (little_endian()) {
        *dest = 0;
        for (uint8_t i = 0; i < 4; ++i) {
            *dest |= (src[i] << (24 - 8 * i));
        }
    } else {
        memcpy(dest, src, 4);
    }
}

void UInt32ToNetwork(uint8_t* dest, const uint32_t src) {
    if (little_endian()) {
        for (uint8_t i = 0; i < 4; ++i) {
            dest[i] = (src >> (24 - 4 * i)) & 0xFF;
        }
    } else {
        memcpy(dest, &src, 4);
    }
}

}  // namespace R51
