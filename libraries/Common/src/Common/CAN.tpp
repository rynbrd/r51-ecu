namespace R51 {
namespace {

using ::Canny::ERR_FIFO;
using ::Canny::ERR_OK;
using ::Canny::Error;

template <typename Frame>
const Frame* extractMessageFrame(const Message& msg) {
    return nullptr;
}

template <>
const Canny::Frame* extractMessageFrame<Canny::Frame>(const Message& msg) {
    if (msg.type() == Message::CAN_FRAME) {
        return &msg.can_frame();
    }
    return nullptr;
}

template <>
const Canny::J1939Message* extractMessageFrame<Canny::J1939Message>(const Message& msg) {
    if (msg.type() == Message::J1939_MESSAGE) {
        return &msg.j1939_message();
    }
    return nullptr;
}

}

template <typename Frame>
CANNode<Frame>::CANNode(Canny::Connection* can, size_t read_buffer_size, size_t write_buffer_size) :
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

template <typename Frame>
CANNode<Frame>::~CANNode() {
    if (read_buffer_ != nullptr) {
        delete[] read_buffer_;
    }
    if (write_buffer_ != nullptr) {
        delete[] write_buffer_;
    }
}

template <typename Frame>
void CANNode<Frame>::handle(const Message& msg) {
    const Frame* frame = extractMessageFrame<Frame>(msg);
    if (frame == nullptr || !writeFilter(*frame)) {
        return;
    }

    Error err = drainWriteBuffer();
    if (err != ERR_FIFO) {
        err = can_->write(*frame);
    }

    if (err == ERR_OK) {
        return;
    }

    if (err == ERR_FIFO && write_buffer_len_ < write_buffer_size_) {
        write_buffer_[(write_buffer_0_ + write_buffer_len_) % write_buffer_size_] = *frame;
        ++write_buffer_len_;
        return;
    }

    onWriteError(err, *frame);
}

template <typename Frame>
void CANNode<Frame>::emit(const Caster::Yield<Message>& yield) {
    drainWriteBuffer();
    fillReadBuffer();
    if (read_buffer_len_ > 0) {
        yield(read_buffer_[read_buffer_0_]);
        read_buffer_0_ = (read_buffer_0_ + 1) % read_buffer_size_;
        --read_buffer_len_;
    }
}

template <typename Frame>
void CANNode<Frame>::fillReadBuffer() {
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

template <typename Frame>
Error CANNode<Frame>::drainWriteBuffer() {
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
