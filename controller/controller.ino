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

CanConnection can(CAN_CS_PIN, CAN_BAUDRATE, CAN_CLOCK);
RealDashConnection realdash;
D(SerialConnection serial);

Connection* connections[] = {
    &can,
    &realdash,
    D(&serial)
};

VehicleClimate climate_system;
VehicleSettings settings_system;
RealDashClimate climate_dash(REALDASH_REPEAT);
RealDashSettings settings_dash(REALDASH_REPEAT);

void connect() {
#ifdef CAN_LISTEN_ONLY
    climate_system.connect(nullptr, &climate_dash);
#else
    climate_system.connect(&can, &climate_dash);
    settings_system.connect(&can, &settings_dash);
#endif
    climate_dash.connect(&realdash, &climate_system);
    settings_dash.connect(&realdash, &settings_system);
}

void receive() {
    climate_system.receive(frame.id, frame.len, frame.data);
    settings_system.receive(frame.id, frame.len, frame.data);
    climate_dash.receive(frame.id, frame.len, frame.data);
    settings_dash.receive(frame.id, frame.len, frame.data);
}

void push() {
    climate_system.push();
    settings_system.push();
    climate_dash.push();
    settings_dash.push();
}

void setup() {
    DEBUG_BEGIN();
    INFO_MSG("setup: initializing ecu");

    D({
        serial.begin(&DEBUG_SERIAL);
#ifdef DEBUG_WAIT_FOR_SERIAL
        while (!DEBUG_SERIAL) {
            delay(100);
        }
#endif
    })

    REALDASH_SERIAL.begin(REALDASH_BAUDRATE);
    while (!REALDASH_SERIAL) {
        delay(100);
    }
    realdash.begin(&REALDASH_SERIAL);

    while (!can.begin()) {
        delay(1000);
    }
    INFO_MSG("setup: ecu started");

    connect();
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
