#ifndef __R51_BINARY_H__
#define __R51_BINARY_H__

#include <Arduino.h>

// Get a bit from a byte array.
bool getBit(const byte* b, uint8_t offset, uint8_t bit);

// Set a bit in a byte array. Return true if the bit was changed.
bool setBit(byte* b, uint8_t offset, uint8_t bit, bool value);

// Check if the same bit in the two bytes differs.
bool xorBits(const byte* b1, const byte* b2, uint8_t offset, uint8_t bit);

// Set a bit if it differs from the current value. Return true if the bit was
// set or false if it was not.
bool setBitXor(byte* b, uint8_t offset, uint8_t bit, bool value);

// Toggle a bit in a byte array. Return the resulting bit.
bool toggleBit(byte* b, uint8_t offset, uint8_t bit);


#endif  // __R51_BINARY_H__
