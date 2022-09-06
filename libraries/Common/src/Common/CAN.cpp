#include "CAN.h"

#include <Canny.h>
#include <Caster.h>

#include "Message.h"

namespace R51 {

using ::Canny::ERR_FIFO;
using ::Canny::ERR_OK;
using ::Canny::Error;
using ::Canny::Frame;

CANNode::CANNode(Canny::Connection* can, size_t read_buffer_size) :
        can_(can), retries_(5), read_buffer_(nullptr),
        read_buffer_size_(read_buffer_size),
        read_buffer_len_(0), read_buffer_0_(0) {
    if (read_buffer_size_ < 1) {
        read_buffer_size_ = 1;
    }
    read_buffer_ = new Frame[read_buffer_size_];
    for (size_t i = 0; i < read_buffer_size_; ++i) {
        read_buffer_[i].reserve(8);
    }
}

CANNode::~CANNode() {
    if (read_buffer_ != nullptr) {
        delete[] read_buffer_;
    }
}

void CANNode::handle(const Message& msg) {
    if (msg.type() != Message::CAN_FRAME || !writeFilter(msg.can_frame())) {
        return;
    }

    Canny::Error err;
    uint8_t attempts = 0;
    do {
        err = can_->write(msg.can_frame());
        attempts++;
    } while (err == ERR_FIFO && attempts <= retries_);

    if (err != ERR_OK) {
        onWriteError(err, msg.can_frame());
    }
}

void CANNode::emit(const Caster::Yield<Message>& yield) {
    fillReadBuffer();
    if (read_buffer_len_ > 0) {
        yield(read_buffer_[read_buffer_0_]);
        read_buffer_0_ = (read_buffer_0_ + 1) % read_buffer_size_;
        --read_buffer_len_;
    }
}

void CANNode::fillReadBuffer() {
    size_t i;
    Error err;
    while (read_buffer_len_ < read_buffer_size_) {
        i = (read_buffer_0_ + read_buffer_len_) % read_buffer_size_;
        err = can_->read(read_buffer_ + i);
        if (err == ERR_FIFO) {
            return;
        } else if (err != ERR_OK) {
            onReadError(err);
            return;
        }
        if (readFilter(read_buffer_[i])) {
            ++read_buffer_len_;
        }
    }
}

}  // namespace R51
