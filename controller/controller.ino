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

CanConnection can(CAN_CS_PIN, CAN_INT_PIN, CAN_BAUDRATE, CAN_CLOCK);
RealDashConnection realdash;
D(SerialConnection serial);

// Initialize CAN masks and filters. We're interested in receiving the
// following standard frames:
//   0x54A (0000010101001010) - climate temperature
//   0x54B (0000010101001011) - climate settings
//   0x72E (0000011100101110) - settings responses
//   0x72F (0000011100101111) - settings responses
// Since we're filtering four frames we can fit the entire set of frames in
// the given filters with a fully set mask. 
bool initFilters() {
    return can.setMask(CanConnection::RXM0, 0xFFFF) &&
        can.setFilter(CanConnection::RXF0, 0x054A) &&
        can.setFilter(CanConnection::RXF1, 0x054B) &&
        can.setMask(CanConnection::RXM1, 0xFFFF) &&
        can.setFilter(CanConnection::RXF2, 0x072E) &&
        can.setFilter(CanConnection::RXF2, 0x072F);
}

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

    while (!initFilters()) {
        delay(100);
    }
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
