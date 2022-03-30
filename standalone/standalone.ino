#include <Arduino.h>

#include "src/bus.h"
#include "src/can.h"
#include "src/climate.h"
#include "src/config.h"
#include "src/debug.h"
#include "src/realdash.h"
#include "src/serial.h"
#include "src/settings.h"
#include "src/steering.h"


class ControllerCan : public Same51Can {
    public:
        // Only send climate and settings frames over CAN.
        bool filter(uint32_t id) const override {
            return (id & 0xFFFFFFFE) == 0x540 ||
                   (id & 0xFFFFFFFE) == 0x71E;
        }
};

class ControllerRealDash : public RealDash {
    public:
        // Only send dashboard state frames to RealDash.
        bool filter(uint32_t id) const override {
            return id == 0x5400 || id == 0x5700 || id == 0x5800;
        }
};

ControllerCan can;
Climate climate;
ControllerRealDash realdash;
Settings settings;
SteeringKeypad steering_keypad;
D(SerialText serial_test);

Bus* bus;
Node* nodes[] = {
    &can,
    &climate,
    &realdash,
    &settings,
    &steering_keypad,
#ifdef DEBUG
    &serial_test,
#endif
};

void setup_debug() {
    DEBUG_BEGIN();
    D(serial_test.begin(&DEBUG_SERIAL));
    #ifdef DEBUG_WAIT_FOR_SERIAL
    WAIT_FOR_SERIAL(DEBUG_SERIAL, 100, nullptr);
    #endif
}

void setup_realdash() {
    INFO_MSG("setup: connecting to realdash");
    REALDASH_SERIAL.begin(REALDASH_BAUDRATE);
    #ifdef REALDASH_WAIT_FOR_SERIAL
    WAIT_FOR_SERIAL(REALDASH_SERIAL, 1000, "setup: dash serial not ready");
    #endif
    realdash.begin(&REALDASH_SERIAL);
}

void setup_can() {
    INFO_MSG("setup: connecting to can bus");
    can.begin();
}

void setup_bus() {
    INFO_MSG("setup: initializing bus");
    bus = new Bus(nodes, sizeof(nodes)/sizeof(nodes[0]));
}

void setup() {
    D(setup_debug());
    setup_realdash();
    setup_can();
    setup_bus();
    INFO_MSG("setup: ecu started");
}

void loop() {
    bus->loop();
}
