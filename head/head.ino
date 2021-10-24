#include "climate.h"
#include "debug.h"
#include "mcp_can.h"
#include "realdash.h"
#include "vehicle.h"

#define SERIAL_BAUDRATE 1000000
#define CAN_CS_PIN 17
#define CAN_BAUDRATE CAN_500KBPS

struct {
    uint32_t id;
    uint8_t len;
    byte data[8];
} frame;

MCP_CAN can(CAN_CS_PIN);
RealDashSerial realdash;

VehicleController vehicle_controller;
VehicleListener vehicle_listener;
RealDashController dash_controller;

void setup() {
    Serial.begin(SERIAL_BAUDRATE);
    while (!Serial) {
        delay(100);
    }

    if (can.begin(CAN_BAUDRATE) != CAN_OK) {
        Debug.println("can init failure");
    }
    realdash.begin(&Serial);

    vehicle_controller.connect(&can);
    vehicle_listener.connect(&dash_controller);
    dash_controller.connect(&realdash);
}

// The loop must push out all state changes for each frame before processing
// another frame. This means that for each data source all controllers need to
// have their push methods called in order to flush state changes to connected
// hardware. Otherwise state changes in the controller could get clobbered by a
// new frame before those changes are pushed out.
void loop() {
    if (can.readMsgBufID(&frame.id, &frame.len, frame.data) == CAN_OK) {
        vehicle_controller.update(frame.id, frame.len, frame.data);
        vehicle_listener.update(frame.id, frame.len, frame.data);
    }
    vehicle_controller.push();
    dash_controller.push();
}
