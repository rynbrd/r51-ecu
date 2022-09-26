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
void CANNode<Frame>::handle(const Message& msg, const Caster::Yield<Message>&) {
    const Frame* frame = extractMessageFrame<Frame>(msg);
    if (frame == nullptr) {
        return;
    }
    Error err = can_->write(*frame);
    if (err != ERR_OK && err != ERR_FIFO) {
        onWriteError(err, *frame);
    }
}

template <typename Frame>
void CANNode<Frame>::emit(const Caster::Yield<Message>& yield) {
    Error err = can_->read(&frame_);
    if (err == ERR_OK) {
        yield(frame_);
    } else if (err != ERR_FIFO) {
        onReadError(err);
    }
}

}  // namespace R51
