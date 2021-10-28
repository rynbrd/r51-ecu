#include "debug.h"

#ifdef DEBUG

#include <Arduino.h>
#include <stdint.h>

size_t printDebugFrame(uint32_t id, uint8_t len, uint8_t* data) {
    size_t n = 0;
    n += DEBUG_SERIAL.print(id, HEX);
    n += DEBUG_SERIAL.print("#");
    for (int i = 0; i < len; i++) {
        if (data[i] <= 0x0F) {
            n += DEBUG_SERIAL.print("0");
        }
        n += DEBUG_SERIAL.print(data[i], HEX);
        if (i < len-1) {
            n += DEBUG_SERIAL.print(":");
        }
    }
    return n;
}

#endif  // DEBUG
