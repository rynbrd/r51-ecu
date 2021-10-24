#include "climate.h"
#include "debug.h"
#include "mcp_can.h"
#include "realdash.h"

#define SERIAL_BAUDRATE 1000000
#define CAN_CS_PIN 17
#define CAN_BAUDRATE CAN_500KBPS

struct {
    uint32_t id;
    uint8_t len;
    byte data[8];
} frame;

ClimateController climate;
MCP_CAN can(CAN_CS_PIN);
RealDash dash;

void setup() {
    Serial.begin(SERIAL_BAUDRATE);
    while (!Serial) {
        delay(100);
    }

    if (can.begin(CAN_BAUDRATE) != CAN_OK) {
        Debug.println("can init failure");
    }
    dash.begin(&Serial);
}

void loop() {
    if (can.readMsgBufID(&frame.id, &frame.len, frame.data) == CAN_OK) {
        climate.update(frame.id, frame.len, frame.data);
    }
    climate.emit(&can);
}
