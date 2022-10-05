#ifndef _R51_BRIDGE_CAN_H_
#define _R51_BRIDGE_CAN_H_

#include "Config.h"

#include <Canny.h>
#include "Debug.h"

namespace R51 {

// CAN connection which filters and buffers frames; and logs errors to serial.
class FilteredCAN : public Canny::BufferedConnection {
    public:
        FilteredCAN(Canny::Connection* can) :
                Canny::BufferedConnection(can, VEHICLE_READ_BUFFER, VEHICLE_WRITE_BUFFER, 8) {}

        // Read R51 climate, settings, tire, and IPDM state frames.
        bool readFilter(const Canny::Frame& frame) const override {
            return (frame.id() & 0xFFFFFFFE) == 0x54A ||
                   (frame.id() & 0xFFFFFFFE) == 0x72E ||
                    frame.id() == 0x385 || frame.id() == 0x625;
        }

        // Write R51 climate and settings control frames.
        bool writeFilter(const Canny::Frame& frame) const override {
            return (frame.id() & 0xFFFFFFFE) == 0x540 ||
                   (frame.id() & 0xFFFFFFFE) == 0x71E;
        }

        // Log read errors to debug serial.
        void onReadError(Canny::Error err) const override {
            DEBUG_MSG_VAL("can: read error: ", err);
        }

        // Log write errors to debug serial.
        void onWriteError(Canny::Error err, const Canny::Frame& frame) const override {
            DEBUG_MSG_VAL("can: write error: ", err);
            DEBUG_MSG_VAL("can: dropped frame: ", frame);
        }
};

}  // namespace R51

#endif  // _R51_BRIDGE_CAN_H_
