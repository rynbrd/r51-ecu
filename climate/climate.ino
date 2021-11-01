#include <Arduino.h>

#include "climate.h"
#include "debug.h"
#include "listener.h"
#include "mcp_can.h"
#include "realdash.h"
#include "serial.h"
#include "vehicle.h"

#define REALDASH_SERIAL Serial1
#define REALDASH_BAUDRATE 115200
#define CAN_CS_PIN 17
#define CAN_BAUDRATE CAN_500KBPS

struct {
    uint32_t id;
    uint8_t len;
    byte data[64];
} frame;

MCP_CAN can(CAN_CS_PIN);
RealDashReceiver realdash;
D(SerialReceiver serial_receiver);

VehicleController vehicle_controller;
VehicleListener vehicle_listener;
RealDash dashboard;

void connect() {
    vehicle_controller.connect(&can);
    vehicle_listener.connect(&dashboard);
    dashboard.connect(&realdash, &vehicle_controller);
}

void receive() {
    vehicle_controller.receive(frame.id, frame.len, frame.data);
    vehicle_listener.receive(frame.id, frame.len, frame.data);
    dashboard.receive(frame.id, frame.len, frame.data);
}

void push() {
    vehicle_controller.push();
    dashboard.push();
}

void setup() {
    DEBUG_BEGIN();
    INFO_MSG("setup: initializing ecu");
    D(serial_receiver.begin(&DEBUG_SERIAL));

    REALDASH_SERIAL.begin(REALDASH_BAUDRATE);
    while (!REALDASH_SERIAL) {
        delay(100);
    }
    realdash.begin(&REALDASH_SERIAL);

    while (can.begin(CAN_BAUDRATE) != CAN_OK) {
        ERROR_MSG("setup: can bus init failure");
        delay(1000);
    }

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
        receive();
    }
    push();

    if (realdash.read(&frame.id, &frame.len, frame.data)) {
        receive();
    }
    push();

    D({
        if (serial_receiver.read(&frame.id, &frame.len, frame.data)) {
            receive();
        }
        push();
    })
}
