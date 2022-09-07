#ifndef _R51_BRIDGE_CAN_H_
#define _R51_BRIDGE_CAN_H_

#include <Canny.h>
#include <Caster.h>
#include <Common.h>

#include "Debug.h"

namespace R51 {

// Read R51 climate, settings, tire, and IPDM state frames.
bool vehicleReadFilter(uint32_t id) {
    return (id & 0xFFFFFFFE) == 0x54A ||
           (id & 0xFFFFFFFE) == 0x72E ||
            id == 0x385 || id == 0x625;
}

// Send R51 climate and settings control frames.
bool vehicleWriteFilter(uint32_t id) {
    return (id & 0xFFFFFFFE) == 0x540 ||
           (id & 0xFFFFFFFE) == 0x71E;
}

class VehicleConnection : public Canny::FilteredConnection {
    public:
        VehicleConnection(Canny::Connection* controller) :
                Canny::FilteredConnection(controller) {}

        // Read R51 climate, settings, tire, and IPDM state frames.
        bool readFilter(uint32_t id, uint8_t, uint8_t*, uint8_t) const override {
            return vehicleReadFilter(id);
        }

        // Send R51 climate and settings control frames.
        bool writeFilter(uint32_t id, uint8_t, uint8_t*, uint8_t) const override {
            return vehicleWriteFilter(id);
        }
};

// CAN node which logs errors.
class LoggingCANNode : public CANNode<Canny::Frame> {
    public:
        LoggingCANNode(Canny::Connection* connection) : CANNode(connection) {}

        // Log read errors to debug serial.
        void onReadError(Canny::Error err) override {
            DEBUG_MSG_VAL("can: read error: ", err);
        }

        // Log write errors to debug serial.
        void onWriteError(Canny::Error err, const Canny::Frame& frame) override {
            DEBUG_MSG_VAL("can: write error: ", err);
            DEBUG_MSG_VAL("can: dropped frame: ", frame);
        }
};

}  // namespace R51

#endif  // _R51_BRIDGE_CAN_H_
