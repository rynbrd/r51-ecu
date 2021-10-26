#include "debug.h"

#include <stdint.h>

#ifdef DEBUG_STREAM

Stream* Debug;

size_t printDebugFrame(uint32_t id, uint8_t len, uint8_t* data) {
    size_t n = 0;
    n += Debug->print(id, HEX);
    n += Debug->print("#");
    for (int i = 0; i < len; i++) {
        if (data[i] <= 0x0F) {
            n += Debug->print("0");
        }
        n += Debug->print(data[i], HEX);
        if (i < len-1) {
            n += Debug->print(":");
        }
    }
    return n;
}

#endif
