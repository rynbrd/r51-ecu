#include <Arduino.h>

#include "can.h"
#include "climate.h"
#include "config.h"
#include "debug.h"
#include "listener.h"
#include "realdash.h"
#include "serial.h"
#include "vehicle.h"

struct {
    uint32_t id;
    uint8_t len;
    byte data[64];
} frame;

CanReceiver can;
RealDashReceiver realdash;
D(SerialReceiver serial_receiver);

Receiver* receivers[] = {
    &can,
    &realdash,
    D(&serial_receiver)
};

VehicleController vehicle_controller;
VehicleListener vehicle_listener;
RealDashController dash_controller(REALDASH_REPEAT);
RealDashListener dash_listener;

void connect() {
#ifndef CAN_LISTEN_ONLY
    vehicle_controller.connect(&can);
#endif
    vehicle_listener.connect(&dash_controller);
    dash_controller.connect(&realdash);
    dash_listener.connect(&vehicle_controller);
}

void receive() {
    vehicle_listener.receive(frame.id, frame.len, frame.data);
    dash_listener.receive(frame.id, frame.len, frame.data);
}

void push() {
    vehicle_controller.push();
    dash_controller.push();
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

    while (!can.begin(CAN_CS_PIN, CAN_BAUDRATE, CAN_CLOCK)) {
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
    for (int i = 0; i < sizeof(receivers)/sizeof(receivers[0]); i++) {
        if (receivers[i]->read(&frame.id, &frame.len, frame.data)) {
            receive();
        }
        push();
    }
}
