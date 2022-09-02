#include "Node.h"

namespace R51 {

void BLENode::handle(const Message& msg) {
    if (msg.type() != Message::EVENT ||  msg.event().subsystem != (uint8_t)SubSystem::BLUETOOTH) {
        return;
    }

    switch ((BluetoothEvent)msg.event().id) {
        case BluetoothEvent::REQUEST:
            emit_ = true;
            break;
        case BluetoothEvent::DISCONNECT:
            Serial.println("disconnected");
            ble_->disconnect();
            break;
        case BluetoothEvent::FORGET:
            ble_->forget();
            break;
        default:
            Serial.println("unrecognized event");
            break;
    }
}

void BLENode::emit(const Caster::Yield<Message>& yield) {
    if (emit_) {
        yield(event_);
    }
}

void BLENode::onConnect() {
    event_.data[0] = 0x01;
    emit_ = true;
}

void BLENode::onDisconnect() {
    event_.data[0] = 0x00;
    emit_ = true;
}

} //  namespace R51
