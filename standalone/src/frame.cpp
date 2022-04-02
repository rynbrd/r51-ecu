#include "frame.h"


bool frameEquals(const Frame& left, const Frame& right) {
    return memcmp(&left, &right, 5+left.len) == 0;
}

void initFrame(Frame* frame, uint32_t id, uint8_t len) {
    frame->id = id;
    frame->len = len;
    memset(frame->data, 0, len);
}

void copyFrame(Frame* dest, const Frame& src) {
    memcpy(dest, &src, sizeof(Frame));
}
