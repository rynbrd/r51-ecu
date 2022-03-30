#include "debug.h"

#include <Arduino.h>
#include "config.h"


#ifdef DEBUG_ENABLE

size_t printDebugFrame(const Frame& frame) {
    size_t n = 0;
    n += DEBUG_SERIAL.print(frame.id, HEX);
    n += DEBUG_SERIAL.print("#");
    for (int i = 0; i < frame.len; i++) {
        if (frame.data[i] <= 0x0F) {
            n += DEBUG_SERIAL.print("0");
        }
        n += DEBUG_SERIAL.print(frame.data[i], HEX);
        if (i < frame.len-1) {
            n += DEBUG_SERIAL.print(":");
        }
    }
    return n;
}

#endif  // DEBUG_ENABLE
