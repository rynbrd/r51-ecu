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

CanConnection can;
RealDashConnection realdash;
D(SerialConnection serial);

Connection* connections[] = {
    &can,
    &realdash,
    D(&serial)
};

VehicleClimate climate;
RealDashClimate dashboard(REALDASH_REPEAT);

void connect() {
#ifdef CAN_LISTEN_ONLY
    climate.connect(nullptr, &dashboard);
#else
    climate.connect(&can, &dashboard);
#endif
    dashboard.connect(&realdash, &climate);
}

void receive() {
    climate.receive(frame.id, frame.len, frame.data);
    dashboard.receive(frame.id, frame.len, frame.data);
}

void push() {
    climate.push();
    dashboard.push();
}

void setup() {
    DEBUG_BEGIN();
    INFO_MSG("setup: initializing ecu");
    D(serial.begin(&DEBUG_SERIAL));

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
    for (uint8_t i = 0; i < sizeof(connections)/sizeof(connections[0]); i++) {
        if (connections[i]->read(&frame.id, &frame.len, frame.data)) {
            receive();
        }
        push();
    }
}
