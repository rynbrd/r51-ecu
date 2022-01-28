#include <Arduino.h>

#include "can.h"
#include "climate.h"
#include "config.h"
#include "debug.h"
#include "listener.h"
#include "realdash.h"
#include "serial.h"
#include "steering.h"
#include "vehicle.h"

struct {
    uint32_t id;
    uint8_t len;
    byte data[64];
} frame;

CanConnection can(CAN_BAUDRATE);
RealDashConnection realdash;
D(SerialConnection serial);

Connection* connections[] = {
    &can,
    &realdash,
    D(&serial)
};

VehicleClimate climate_system(REAR_DEFROST_PIN, REAR_DEFROST_TRIGGER_MS);
VehicleSettings settings_system;
RealDashClimate climate_dash(REALDASH_REPEAT);
RealDashSettings settings_dash(REALDASH_REPEAT);
RealDashKeypad dash_keypad(REALDASH_REPEAT);
SteeringSwitch steering_switch(STEERING_SWITCH_A_PIN, STEERING_SWITCH_B_PIN);

void connect() {
#ifdef CAN_LISTEN_ONLY
    climate_system.connect(nullptr, &climate_dash);
#else
    climate_system.connect(&can, &climate_dash);
    settings_system.connect(&can, &settings_dash);
#endif
    climate_dash.connect(&realdash, &climate_system);
    settings_dash.connect(&realdash, &settings_system);
    dash_keypad.connect(&realdash);
    steering_switch.connect(&dash_keypad);
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
    dash_keypad.push();
}

void setup() {
    DEBUG_BEGIN();

    D(serial.begin(&DEBUG_SERIAL));
    #ifdef DEBUG_WAIT_FOR_SERIAL
    WAIT_FOR_SERIAL(REALDASH_SERIAL, 100, nullptr);
    #endif

    INFO_MSG("setup: connecting to dash");
    REALDASH_SERIAL.begin(REALDASH_BAUDRATE);
    #ifdef REALDASH_WAIT_FOR_SERIAL
    WAIT_FOR_SERIAL(REALDASH_SERIAL, 1000, "setup: dash serial not ready");
    #endif
    realdash.begin(&REALDASH_SERIAL);

    INFO_MSG("setup: connecting to vehicle");
    can.begin();

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
    steering_switch.read();
    push();
}
