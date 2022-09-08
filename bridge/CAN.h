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

// J1939 connection which filters and buffers frames; and logs errors to serial.
class FilteredJ1939 : public Canny::BufferedConnection {
    public:
        FilteredJ1939(Canny::Connection* can, uint8_t address) :
                Canny::BufferedConnection(can, J1939_READ_BUFFER, J1939_WRITE_BUFFER, 8),
                address_(address) {}

        // Read broadcast messages and messages addressed to this device.
        bool readFilter(const Canny::Frame& frame) const override {
            const Canny::J1939Message* j1939 = (Canny::J1939Message*)(&frame);
            return j1939->broadcast() || j1939->dest_address() == address_;
        }

        // Write messages sent from this device.
        bool writeFilter(const Canny::Frame& frame) const override {
            const Canny::J1939Message* j1939 = (Canny::J1939Message*)(&frame);
            if (!j1939->valid()) {
                onWriteError(Canny::Error::ERR_INVALID, frame);
                return false;
            }
            return j1939->source_address() == address_;
        }

        // Log read errors to debug serial.
        void onReadError(Canny::Error err) const override {
            DEBUG_MSG_VAL("j1939: read error: ", err);
        }

        // Log write errors to debug serial.
        void onWriteError(Canny::Error err, const Canny::Frame& frame) const override {
            DEBUG_MSG_VAL("j1939: write error: ", err);
            DEBUG_MSG_VAL("j1939: dropped frame: ", frame);
        }

    private:
        uint8_t address_;
};

}  // namespace R51

#endif  // _R51_BRIDGE_CAN_H_
