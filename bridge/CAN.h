#ifndef _R51_BRIDGE_NODES_
#define _R51_BRIDGE_NODES_

#include <Canny.h>
#include <Caster.h>
#include <Common.h>

#include "Debug.h"

// CAN node which filters relevant R51 vehicle frames.
class FilteredCAN : public R51::CANNode {
    public:
        FilteredCAN() : R51::CANNode(&CAN) {}

        // Read R51 climate and settings state frames.
        bool readFilter(const Canny::Frame& frame) const override {
            return (frame.id() & 0xFFFFFFFE) == 0x54A ||
                   (frame.id() & 0xFFFFFFFE) == 0x72E ||
                    frame.id() == 0x625;
        }

        // Send R51 climate and settings control frames.
        bool writeFilter(const Canny::Frame& frame) const override {
            return (frame.id() & 0xFFFFFFFE) == 0x540 ||
                   (frame.id() & 0xFFFFFFFE) == 0x71E;
        }

        // Log read errors to debug serial.
        void onReadError(Canny::Error err) override {
            ERROR_MSG_VAL("can: read error: ", err);
        }

        // Log write errors to debug serial.
        void onWriteError(Canny::Error err, const Canny::Frame& frame) override {
            ERROR_MSG_VAL("can: write error: ", err);
            ERROR_MSG_VAL("can: dropped frame: ", frame);
        }
};

#endif  // _R51_BRIDGE_NODES_
