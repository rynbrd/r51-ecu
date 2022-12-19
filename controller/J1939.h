#ifndef _R51_CONTROLLER_J1939_H_
#define _R51_CONTROLLER_J1939_H_

#include "Config.h"

#include <Canny.h>
#include <Canny/MCP2515.h>
#include <Core.h>
#include "Debug.h"

Canny::MCP2515<Canny::J1939Message> J1939(MCP2515_CS_PIN);

namespace R51 {

// J1939 connection which that logs errors to serial.
// TODO: Enable MCP hardware filtering.
// TODO: Check if MCP hardware is wired for interrupts.
class J1939Connection : public Canny::BufferedConnection<Canny::J1939Message> {
    public:
        J1939Connection() :
            Canny::BufferedConnection<Canny::J1939Message>(
                    &J1939, J1939_READ_BUFFER, J1939_WRITE_BUFFER) {}

        // Log read errors to debug serial.
        void onReadError(Canny::Error err) const override {
            DEBUG_MSG_VAL("j1939: read error: ", err);
        }

        // Log write errors to debug serial.
        void onWriteError(Canny::Error err, const Canny::J1939Message& msg) const override {
            DEBUG_MSG_VAL("j1939: write error: ", err);
            DEBUG_MSG_OBJ("j1939: dropped frame: ", msg);
        }
};

class J1939ControllerAdapter : public J1939Adapter {
    public:
        // Route bridge and vehicle specific events to the bridge ECU.
        // Broadcast state events. Filter everything else.
        uint8_t route(const Event& event) override {
            if ((event.subsystem >= 0x10 && event.subsystem <= 0x1F) ||
                    event.subsystem == (uint8_t)SubSystem::BLUETOOTH) {
                return BRIDGE_ADDRESS;
            } else if ((event.id & 0xF0) == 0x00) {
                return 0xFF;
            }
            return Canny::NullAddress;
        }
};

}  // namespace R51

#endif  // _R51_CONTROLLER_J1939_H_
