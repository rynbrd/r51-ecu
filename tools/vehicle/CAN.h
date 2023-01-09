#ifndef _R51_TOOLS_VEHICLE_CAN_H_
#define _R51_TOOLS_VEHICLE_CAN_H_

#include "Config.h"

#include <Canny.h>
#include <Canny/MCP2515.h>
#include "Debug.h"

Canny::MCP2515<Canny::CAN20Frame> CAN(MCP2515_CS_PIN);

namespace R51 {

// CAN connection which filters and buffers frames; and logs errors to serial.
class CANConnection : public Canny::BufferedConnection<Canny::CAN20Frame> {
    public:
        CANConnection() :
            Canny::BufferedConnection<Canny::CAN20Frame>(
                    &CAN, VEHICLE_READ_BUFFER, VEHICLE_WRITE_BUFFER) {
            // Enable hardware filtering if not in promiscuous mode.
            if (!VEHICLE_PROMISCUOUS) {
                // Read climate and settings frames.
                CAN.setMask(0, 0, 0xFFFFFFFE);
                CAN.setFilter(0, 0, 0x54A);
                CAN.setFilter(1, 0, 0x72E);
                // Read tire and IPDM frames.
                CAN.setMask(1, 0, 0xFFFFFFFF);
                CAN.setFilter(3, 0, 0x385);
                CAN.setFilter(2, 0, 0x625);
            }
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

#endif  // _R51_TOOLS_VEHICLE_CAN_H_
