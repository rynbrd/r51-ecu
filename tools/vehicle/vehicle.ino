#include "Config.h"
#include "Debug.h"

#include <Arduino.h>
#include <Caster.h>
#include <Console.h>
#include <Core.h>
#include <Platform.h>
#include <Vehicle.h>
#include "CAN.h"

using namespace ::R51;
using ::Caster::Bus;
using ::Caster::Node;

// Flash Config Storate
PlatformConfigStore config;

// Vehicle CAN Connectivity
CANConnection can_conn;
CANGateway can_gw(&can_conn);

// Vehicle System Integrations
Defrost defrost(DEFROST_HEATER_PIN, DEFROST_HEATER_MS);
SteeringKeypad steering_keypad(STEERING_KEYPAD_ID, STEERING_PIN_A, STEERING_PIN_B);
Climate climate;
Settings settings;
IPDM ipdm;
TirePressure tire_pressure(&config);
Illum illum;

// Serial Console
ConsoleNode console(&SERIAL_DEVICE, false);

// Internal Bus
Node<Message>* nodes[] = {
    &can_gw,
    &defrost,
    &steering_keypad,
    &climate,
    &settings,
    &ipdm,
    &tire_pressure,
    &illum,
};
Bus<Message> bus(nodes, sizeof(nodes)/sizeof(nodes[0]));

void setup() {
    SERIAL_DEVICE.begin(SERIAL_BAUDRATE);
    if (SERIAL_WAIT) {
        while(!SERIAL_DEVICE) {
            delay(100);
        }
    }
    DEBUG_MSG("setup: initializing ECU");

    while (!CAN.begin(VEHICLE_CAN_MODE)) {
        DEBUG_MSG("setup: failed to connect to CAN");
        delay(500);
    }

    bus.init();

    DEBUG_MSG("setup: ECU started");
}

void loop() {
    bus.loop();
    delay(10);
}
