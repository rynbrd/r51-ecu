#ifndef _R51_BLUETOOTH_NODE_H_
#define _R51_BLUETOOTH_NODE_H_

#include <Caster.h>
#include <Core.h>
#include "BLE.h"

namespace R51 {

enum class BluetoothEvent : uint8_t {
    STATE   = 0x00, // State event. Holds the current connection state.

    DISCONNECT_CMD  = 0x10, // Disconnect the current host device.
    FORGET_CMD      = 0x11, // Disconnect and forget the current host device.
};

// Node for managing BLE connectivity.
class BLENode : public Caster::Node<Message> {
    public:
        BLENode(BLE* ble) :
            ble_(ble),
            event_((uint8_t)SubSystem::BLUETOOTH, (uint8_t)BluetoothEvent::STATE, {0x00}),
            emit_(false) {}

        void handle(const Message& msg, const Caster::Yield<Message>& yield) override;

        void emit(const Caster::Yield<Message>& yield) override;

        // Should be called when the BLE host device connects. 
        void onConnect();

        // Should be called when the BLE host device disconnects.
        void onDisconnect();

    private:
        BLE* ble_;
        Event event_;
        bool emit_;
};

}  // namespace R51

#endif  // _R51_BLUETOOTH_SUBSYSTEM_H_
