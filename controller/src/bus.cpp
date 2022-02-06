#include "bus.h"

bool framesEqual(Frame* left, Frame* right) {
    if (left != nullptr && right != nullptr) {
        return memcmp(left, right, 5+left->len) == 0;
    }
    return left == right;
}

void Bus::loop() {
    for (int i = 0; i < count_; i++) {
        if (nodes_[i]->recieve(&frame_)) {
            broadcast();
        }
    }
}

void Bus::broadcast() {
    for (int i = 0; i < count_; i++) {
        if (nodes_[i]->filter(frame_.id)) {
            nodes_[i]->send(&frame_);
        }
    }
}
