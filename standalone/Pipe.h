#ifndef _R51_STANDALONE_PIPE_H_
#define _R51_STANDALONE_PIPE_H_

#include <Arduino.h>
#include <Core.h>
#include <Platform.h>
#include "Config.h"

namespace R51 {

class FilteredPipe : public Pipe {
    public:
        FilteredPipe() : Pipe(IO_CORE_BUFFER_SIZE, PROC_CORE_BUFFER_SIZE) {}

        // Filtering for the I/O core. Forwards all frames to the processing core.
        bool filterLeft(const Message&) override { return true; }

        // Filtering for the processing core. Forwards CAN frames, J1939
        // messages, and events for BLE serial.
        bool filterRight(const Message& msg) override {
            return msg.type() == Message::CAN_FRAME ||
                msg.type() == Message::J1939_MESSAGE ||
                isBluetoothEvent(msg);
        }

        void onBufferOverrun(const Message& msg) override {
            DEBUG_MSG_OBJ("pipe: dropped frame: ", msg);
        }

    private:
        // Return true if an event should be sent over BLE serial.
        bool isBluetoothEvent(const Message& msg) {
            if (msg.type() != Message::EVENT) {
                return false;
            }

            // Only power system state events.
            switch ((SubSystem)msg.event()->subsystem) {
                // Bluetooth control events.
                case SubSystem::BLUETOOTH:
                    return msg.event()->id >= 0x10;

                // Rotary encoder control events.
                case SubSystem::KEYPAD:
                    return msg.event()->data[0] == ROTARY_ENCODER_ID;

                // Power state events.
                case SubSystem::IPDM:
                case SubSystem::BCM:
                case SubSystem::CLIMATE:
                case SubSystem::POWER:
                    return msg.event()->id < 0x10;
                default:
                    break;
            }
            return false;
        }
};

}  // namespace R51

#endif  // _R51_STANDALONE_PIPE_H_
