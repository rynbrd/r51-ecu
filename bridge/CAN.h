#ifndef _R51_BRIDGE_CAN_H_
#define _R51_BRIDGE_CAN_H_

#include <Canny.h>
#include <Caster.h>
#include <Common.h>

#include "Debug.h"

namespace R51 {

// CAN node which logs errors.
class FilteredCANNode : public CANNode<Canny::Frame> {
    public:
        FilteredCANNode(Canny::Connection* connection) : CANNode(connection) {}

        // Read R51 climate, settings, tire, and IPDM state frames.
        bool readFilter(const Canny::Frame& frame) {
            return (frame.id() & 0xFFFFFFFE) == 0x54A ||
                   (frame.id() & 0xFFFFFFFE) == 0x72E ||
                    frame.id() == 0x385 || frame.id() == 0x625;
        }

        // Send R51 climate and settings control frames.
        bool writeFilter(const Canny::Frame& frame) {
            return (frame.id() & 0xFFFFFFFE) == 0x540 ||
                   (frame.id() & 0xFFFFFFFE) == 0x71E;
        }

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
