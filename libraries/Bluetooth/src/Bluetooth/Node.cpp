#include "Node.h"

namespace R51 {

void BLENode::handle(const Message& msg, const Caster::Yield<Message>&) {
    if (msg.type() != Message::EVENT ||  msg.event().subsystem != (uint8_t)SubSystem::BLUETOOTH) {
        return;
    }

    switch ((BluetoothEvent)msg.event().id) {
        case BluetoothEvent::REQUEST:
            emit_ = true;
            break;
        case BluetoothEvent::DISCONNECT:
            ble_->disconnect();
            break;
        case BluetoothEvent::FORGET:
            ble_->forget();
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
