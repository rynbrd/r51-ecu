#ifndef __R51_FRAME_H__
#define __R51_FRAME_H__

#include <Arduino.h>


// A data frame.
struct Frame {
    uint32_t id;
    uint8_t len;
    byte data[64];
};

// Check if two frames are equal.
bool frameEquals(const Frame& left, const Frame& right);

// Reset a frame. Set the frame's ID and length and zero out the data bytes.
void initFrame(Frame* frame, uint32_t id, uint8_t len);

// Copy contents of frame src to dest.
void copyFrame(Frame* dest, const Frame& src);

#endif  // __R51_FRAME_H__
