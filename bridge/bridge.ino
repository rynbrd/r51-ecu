#include "Config.h"

#include <Arduino.h>
#include <Canny.h>
#include <Canny/Detect.h>
#include <Canny/RealDash.h>
#include <Common.h>
#include <Vehicle.h>

#ifdef BLUETOOTH_ENABLE
#include <Bluetooth.h>
#endif

#include "CAN.h"
#include "RealDash.h"

using ::Caster::Bus;
using ::Caster::Node;
using ::R51::Message;

FilteredCAN can;
R51::Climate climate;
R51::Settings settings;
R51::IPDM ipdm;

#ifdef DEFOG_HEATER_ENABLE
R51::Defog defog(DEFOG_HEATER_PIN, DEFOG_HEATER_MS);
#endif

#ifdef STEERING_KEYPAD_ENABLE
R51::SteeringKeypad steering_keypad(STEERING_PIN_A, STEERING_PIN_B);
#endif

#ifdef BLUETOOTH_ENABLE
R51::BLE ble(BLUETOOTH_SPI_CS_PIN, BLUETOOTH_SPI_IRQ_PIN);
#endif

#ifdef REALDASH_ENABLE
Canny::RealDash realdash_connection(&REALDASH_SERIAL);
RealDashAdapter realdash(&realdash_connection, REALDASH_FRAME_ID);
#endif

Bus<Message>* bus;
Node<Message>* nodes[] = {
    &can,
    &climate,
    &settings,
    &ipdm,
#ifdef DEFOG_HEATER_ENABLE
    &defog,
#endif
#ifdef STEERING_KEYPAD_ENABLE
    &steering_keypad,
#endif
#ifdef REALDASH_ENABLE
    &realdash,
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
        delay(200);
    }
}

void setup_bluetooth() {
#ifdef BLUETOOTH_ENABLE
    INFO_MSG("setup: initializing bluetooth");
    while (!ble.begin()) {
        ERROR_MSG("setup: failed to init bluetooth");
        delay(500);
    }
#endif
}

void setup_bus() {
    INFO_MSG("setup: initializing bus");
    bus = new Bus<Message>(nodes, sizeof(nodes)/sizeof(nodes[0]));
}

void setup() {
    setup_debug();
    setup_can();
    setup_bluetooth();
    setup_bus();
    INFO_MSG("setup: ecu started");
}

void loop() {
    bus->loop();
}
