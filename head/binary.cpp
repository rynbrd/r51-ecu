#include "binary.h"

bool getBit(byte* b, uint8_t offset, uint8_t bit) {
    return ((b[offset] >> bit) & 1) == 1;
}

void setBit(byte* b, uint8_t offset, uint8_t bit, bool value) {
    if (value) {
        b[offset] |= 1 << bit;
    } else {
        b[offset] &= ~(1 << bit);
    }
}

void toggleBit(byte* b, uint8_t offset, uint8_t bit) {
    b[offset] ^= 1 << bit;
}
