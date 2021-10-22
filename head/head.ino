
#include "can.h"
#include "debug.h"
#include "mcp2515.h"

using ECU::Debug;
using ECU::Frame;
using ECU::Mcp2515;
using ECU::Status;

Frame frame;
Mcp2515 mcp(17, ECU::CAN_SPEED_500K);

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }
    Debug.begin(&Serial);

    if (mcp.begin() != ECU::OK) {
        Debug.println("initialization error");
    } else {
        Debug.println("initialized");
    }
}

void loop() {
    Status status = mcp.receive(&frame);
    if (status == ECU::OK) {
        Debug.println(frame);
    } else {
        Debug.print("receieve error: ");
        Debug.println(status);
        delay(200);
    }
}
