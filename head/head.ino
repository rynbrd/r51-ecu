#include <Arduino.h>

// Replace with something reasonable to enable debug logging.
//#define DEBUG_STREAM Serial

#include "debug.h"
#include "climate.h"
#include "listener.h"
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
RealDashListener dash_listener;

void connect() {
    vehicle_controller.connect(&can);
    vehicle_listener.connect(&dash_controller);
    dash_controller.connect(&realdash);
    dash_listener.connect(&vehicle_controller);
}

void receive(uint32_t id, uint8_t len, byte* data) {
    vehicle_controller.receive(frame.id, frame.len, frame.data);
    vehicle_listener.receive(frame.id, frame.len, frame.data);
    dash_listener.receive(frame.id, frame.len, frame.data);
}

void push() {
    vehicle_controller.push();
    dash_controller.push();
}

void setup() {
    Serial.begin(SERIAL_BAUDRATE);
    while (!Serial) {
        delay(100);
    }

    while (can.begin(CAN_BAUDRATE) != CAN_OK) {
        ERROR_MSG("setup: can bus init failure");
        delay(1000);
    }
    realdash.begin(&Serial);

    connect();
    INFO_MSG("setup: ecu started");
}

// The loop must push out all state changes for each frame before processing
// another frame. This means that for each data source all controllers need to
// have their push methods called in order to flush state changes to connected
// hardware. Otherwise state changes in the controller could get clobbered by a
// new frame before those changes are pushed out.
void loop() {
    if (can.readMsgBufID(&frame.id, &frame.len, frame.data) == CAN_OK) {
        receive(frame.id, frame.len, frame.data);
    }
    push();

    if (realdash.read(&frame.id, &frame.len, frame.data)) {
        receive(frame.id, frame.len, frame.data);
    }
    push();
}
