#include "bus.h"

bool framesEqual(Frame* left, Frame* right) {
    if (left != nullptr && right != nullptr) {
        return memcmp(left, right, 5+left->len) == 0;
    }
    return left == right;
}

void Bus::loop() {
    for (uint8_t i = 0; i < count_; i++) {
        if (nodes_[i]->receive(&frame_)) {
            broadcast();
        }
    }
}

void Bus::broadcast() {
    for (uint8_t i = 0; i < count_; i++) {
        if (nodes_[i]->filter(frame_.id)) {
            nodes_[i]->send(&frame_);
        }
    }
}
