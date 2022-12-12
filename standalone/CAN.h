#ifndef _R51_STANDALONE_CAN_H_
#define _R51_STANDALONE_CAN_H_

#include "Config.h"

#include <Canny.h>
#include <Canny/MCP2518.h>
#include "Debug.h"

Canny::MCP2518<Canny::CAN20Frame> CAN(MCP2518_CS_PIN);

namespace R51 {

// CAN connection which filters and buffers frames; and logs errors to serial.
// TODO: Enable MCP hardware filtering.
// TODO: Check if MCP hardware is wired for interrupts.
class CANConnection : public Canny::BufferedConnection<Canny::CAN20Frame> {
    public:
        CANConnection() :
            Canny::BufferedConnection<Canny::CAN20Frame>(
                    &CAN, VEHICLE_READ_BUFFER, VEHICLE_WRITE_BUFFER) {}

        // Read R51 climate, settings, tire, and IPDM state frames.
        bool readFilter(const Canny::CAN20Frame& frame) const override {
            return (frame.id() & 0xFFFFFFFE) == 0x54A ||
                   (frame.id() & 0xFFFFFFFE) == 0x72E ||
                    frame.id() == 0x385 || frame.id() == 0x625;
        }

        // Write R51 climate and settings control frames.
        bool writeFilter(const Canny::CAN20Frame& frame) const override {
            return (frame.id() & 0xFFFFFFFE) == 0x540 ||
                   (frame.id() & 0xFFFFFFFE) == 0x71E;
        }

        // Log read errors to debug serial.
        void onReadError(Canny::Error err) const override {
            DEBUG_MSG_VAL("can: read error: ", err);
        }

        // Log write errors to debug serial.
        void onWriteError(Canny::Error err, const Canny::CAN20Frame& frame) const override {
            DEBUG_MSG_VAL("can: write error: ", err);
            DEBUG_MSG_OBJ("can: dropped frame: ", frame);
        }
};

}  // namespace R51

#endif  // _R51_STANDALONE_CAN_H_
