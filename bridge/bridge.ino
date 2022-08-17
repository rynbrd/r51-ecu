#include <Arduino.h>
#include <Canny.h>
#include <Canny/Detect.h>
#include <R51Core.h>
#include <R51Vehicle.h>

#include "Nodes.h"

using ::Caster::Bus;
using ::Caster::Node;
using ::R51::Message;

FilteredCAN can;
R51::Climate climate;
R51::Settings settings;
R51::IPDM ipdm;

#ifdef DEFOG_HEATER_ENABLE
R51::Defog defog(DEFOG_HEATER_PIN, DEFOGER_HEATER_MS);
#endif

#ifdef STEERING_KEYPAD_ENABLE
R51::SteeringKeypad steering_keypad(STEERING_PIN_A, STEERING_PIN_B);
#endif

Bus<Message>* bus;
Node<Message>* nodes[] = {
    &can,
    &climate,
    &settings,
    &ipdm;
#ifdef DEFOG_HEATER_ENABLE
    &defog,
#endif
#ifdef STEERING_KEYPAD_ENABLE
    &steering_keypad,
#endif
};

void setup_debug() {
#ifdef DEBUG
    DEBUG_BEGIN();
    while(!DEBUG_SERIAL) {
        delay(100);
    }
#endif
}

void setup_can() {
    INFO_MSG("setup: connecting to can bus");
    while (!CAN.begin(Canny::CAN20_250K)) {
        ERROR_MSG("setup: failed to init can bus");
        delay(500);
    }
}

void setup_bus() {
    INFO_MSG("setup: initializing bus");
    bus = new Bus<Message>(nodes, sizeof(nodes)/sizeof(nodes[0]));
}

void setup() {
    setup_debug();
    setup_can();
    setup_bus();
    INFO_MSG("setup: ecu started");
}

void loop() {
    bus->loop();
}
