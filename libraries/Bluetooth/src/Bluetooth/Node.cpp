#include "Node.h"

namespace R51 {

void BLENode::handle(const Message& msg, const Caster::Yield<Message>& yield) {
    if (msg.type() != Message::EVENT) {
        return;
    }
    switch ((SubSystem)msg.event()->subsystem) {
        case SubSystem::CONTROLLER:
            if (RequestCommand::match(*msg.event(), SubSystem::BLUETOOTH,
                    (uint8_t)BluetoothEvent::STATE)) {
                yield(event_);
                emit_ = false;
            }
            break;
        case SubSystem::BLUETOOTH:
            switch ((BluetoothEvent)msg.event()->id) {
                case BluetoothEvent::DISCONNECT_CMD:
                    ble_->disconnect();
                    break;
                case BluetoothEvent::FORGET_CMD:
                    ble_->forget();
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void BLENode::emit(const Caster::Yield<Message>& yield) {
    if (emit_) {
        yield(event_);
        emit_ = false;
    }
}

void BLENode::onConnect() {
    if (event_.data[0] != 0x01) {
        event_.data[0] = 0x01;
        emit_ = true;
    }
}

void BLENode::onDisconnect() {
    if (event_.data[0] != 0x00) {
        event_.data[0] = 0x00;
        emit_ = true;
    }
}

} //  namespace R51
