#include "CAN.h"

#include <Canny.h>
#include <Caster.h>

#include "Message.h"

namespace R51 {

using ::Canny::ERR_FIFO;
using ::Canny::ERR_OK;
using ::Canny::Error;
using ::Canny::Frame;

CANNode::CANNode(Canny::Connection* can, size_t read_buffer_size, size_t write_buffer_size) :
        can_(can), retries_(5), read_buffer_(nullptr),
        read_buffer_size_(read_buffer_size),
        read_buffer_len_(0), read_buffer_0_(0),
        write_buffer_(nullptr), write_buffer_size_(write_buffer_size),
        write_buffer_len_(0), write_buffer_0_(0) {
    if (read_buffer_size_ < 1) {
        read_buffer_size_ = 1;
    }
    read_buffer_ = new Frame[read_buffer_size_];
    for (size_t i = 0; i < read_buffer_size_; ++i) {
        read_buffer_[i].reserve(8);
    }

    if (write_buffer_size_ > 0) {
        write_buffer_ = new Frame[write_buffer_size_];
    }
    for (size_t i = 0; i < write_buffer_size_; ++i) {
        write_buffer_[i].reserve(8);
    }
}

CANNode::~CANNode() {
    if (read_buffer_ != nullptr) {
        delete[] read_buffer_;
    }
    if (write_buffer_ != nullptr) {
        delete[] write_buffer_;
    }
}

void CANNode::handle(const Message& msg) {
    if (msg.type() != Message::CAN_FRAME || !writeFilter(msg.can_frame())) {
        return;
    }

    Error err = drainWriteBuffer();
    if (err != ERR_FIFO) {
        err = can_->write(msg.can_frame());
    }

    if (err == ERR_OK) {
        return;
    }

    if (err == ERR_FIFO && write_buffer_len_ < write_buffer_size_) {
        write_buffer_[(write_buffer_0_ + write_buffer_len_) % write_buffer_size_] = msg.can_frame();
        ++write_buffer_len_;
        return;
    }

    onWriteError(err, msg.can_frame());
}

void CANNode::emit(const Caster::Yield<Message>& yield) {
    drainWriteBuffer();
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

Error CANNode::drainWriteBuffer() {
    Error err = ERR_OK;
    while (write_buffer_len_ > 0) {
        err = can_->write(write_buffer_[write_buffer_0_]);
        if (err == ERR_FIFO) {
            // No write capacity at this time.
            return ERR_FIFO;
        }
        if (err != ERR_OK) {
            // Discard bad frames.
            onWriteError(err, write_buffer_[write_buffer_0_]);
        }
        write_buffer_0_ = (write_buffer_0_ + 1) % write_buffer_size_;
        --write_buffer_len_;
    }
    return ERR_OK;
}

}  // namespace R51
