#include "binary.h"

bool getBit(byte* b, uint8_t offset, uint8_t bit) {
    return ((b[offset] >> bit) & 1) == 1;
}

void setBit(byte* b, uint8_t offset, uint8_t bit, bool value) {
    if (value) {
        b[offset] |= (1 << bit);
    } else {
        b[offset] &= ~(1 << bit);
    }
}

bool xorBits(byte* b1, byte* b2, uint8_t offset, uint8_t bit) {
    return (b1[offset] >> bit & 1) != (b2[offset] >> bit & 1);
}

bool setBitXor(byte* b, uint8_t offset, uint8_t bit, bool value) {
  if ((b[offset] >> bit ^ (byte)value) != 0) {
    b[offset] ^= (1 << bit);
    return true;
  }
  return false;
}

bool toggleBit(byte* b, uint8_t offset, uint8_t bit) {
    return b[offset] ^= (1 << bit);
}
