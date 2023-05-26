#ifndef _R51_TOOLS_CAN_MONITOR_CAN_H_
#define _R51_TOOLS_CAN_MONITOR_CAN_H_

#include "Config.h"

#include <Canny.h>
#include <Canny/MCP2515.h>

Canny::MCP2515<Canny::CAN20Frame> CAN(MCP2515_CS_PIN);

namespace R51 {

// CAN connection which filters and buffers frames; and logs errors to serial.
class CANConnection : public Canny::BufferedConnection<Canny::CAN20Frame> {
    public:
        CANConnection() :
            Canny::BufferedConnection<Canny::CAN20Frame>(
                    &CAN, CAN_READ_BUFFER, CAN_WRITE_BUFFER) {}

        bool begin() {
            // Initialize controller.
            return CAN.begin(CAN_MODE);
        }

        // Log read errors to debug serial.
        void onReadError(Canny::Error err) const override {
            Serial.print("can: read error: ");
            Serial.println(err);
        }

        // Log write errors to debug serial.
        void onWriteError(Canny::Error err, const Canny::CAN20Frame& frame) const override {
            Serial.print("can: write error: ");
            Serial.println(err);
            Serial.print("can: dropped frame: ");
            frame.printTo(Serial);
            Serial.println();
        }
};

}  // namespace R51

#endif  // _R51_TOOLS_CAN_MONITOR_CAN_H_
