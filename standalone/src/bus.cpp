#include "bus.h"


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

void Bus::loop() {
    for (uint8_t i = 0; i < count_; i++) {
        nodes_[i]->receive(broadcast_);
    }
}

void Bus::BroadcastImpl::operator()(const Frame& frame) const {
    for (uint8_t i = 0; i < bus_->count_; i++) {
        if (bus_->nodes_[i]->filter(frame.id)) {
            bus_->nodes_[i]->send(frame);
        }
    }
}
