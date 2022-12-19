#include "Pipe.h"

#include <Arduino.h>

#ifdef PICO_RP2040

#include <Caster.h>
#include <Core.h>

extern "C" {
    #include <pico/util/queue.h>
};

namespace R51 {

void PicoPipeNode::handle(const Message& msg, const Caster::Yield<Message>&) {
    if (!filter(msg)) {
        return;
    }
    MessageValue value(msg);
    if (!queue_try_add(write_queue(), &value)) {
        Serial.print("overrun ");
        parent_->onBufferOverrun(msg);
    } else {
        Serial.print("write ");
    }
    Serial.println(msg);
}

void PicoPipeNode::emit(const Caster::Yield<Message>& yield) {
    MessageValue msg;
    if (queue_try_remove(read_queue(), &msg)) {
        yield(msg);
    }
}

queue_t* PicoPipeNode::read_queue() const {
    if (side_ <= 0) {
        return &parent_->left_queue_;
    }
    return &parent_->right_queue_;
}

queue_t* PicoPipeNode::write_queue() const {
    if (side_ <= 0) {
        return &parent_->right_queue_;
    }
    return &parent_->left_queue_;
}

bool PicoPipeNode::filter(const Message& msg) {
    if (side_ <= 0) {
        return parent_->filterLeft(msg);
    }
    return parent_->filterRight(msg);
}

PicoPipe::PicoPipe(size_t left_capacity, size_t right_capacity) :
        left_node_(this, -1), right_node_(this, +1) {
    queue_init(&left_queue_, sizeof(MessageValue), left_capacity);
    queue_init(&right_queue_, sizeof(MessageValue), right_capacity);
}

PicoPipe::~PicoPipe() {
    queue_free(&left_queue_);
    queue_free(&right_queue_);
}

}  // namespace R51

#endif  // ARDUINO_ARCH_RP2040
