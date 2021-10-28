#ifndef __R51_BINARY_H__
#define __R51_BINARY_H__

#include <Arduino.h>

// Get a bit from a byte array.
bool getBit(byte* b, uint8_t offset, uint8_t bit);

// Set a bit in a byte array.
void setBit(byte* b, uint8_t offset, uint8_t bit, bool value);

// Toggle a bit in a byte array.
void toggleBit(byte* b, uint8_t offset, uint8_t bit);

#endif  // __R51_BINARY_H__
