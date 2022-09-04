#include "Config.h"

#include <Arduino.h>
#include <Canny.h>
#include <Canny/Detect.h>
#include <Canny/RealDash.h>
#include <Common.h>
#include <Vehicle.h>

#if defined(CONSOLE_ENABLE) && defined(SERIAL_DEVICE)
#pragma message("serial console enabled")
#include <Console.h>
#endif

#ifdef BLUETOOTH_ENABLE
#pragma message("bluetooth enabled")
#include <Bluetooth.h>
#endif

#include "CAN.h"
#include "RP2040.h"
#include "RealDash.h"

using ::Caster::Bus;
using ::Caster::Node;
using ::R51::Message;

FilteredCAN can;
R51::Climate climate;
R51::Settings settings;
R51::IPDM ipdm;

#ifdef RASPBERRYPI_PICO
#pragma message("RP2040 support enabled")
PicoConfigStore config(false);
R51::TirePressureState tires(&config);
#else
R51::TirePressureState tires;
#endif

#if defined(CONSOLE_ENABLE) && defined(SERIAL_DEVICE)
R51::ConsoleNode console(&SERIAL_DEVICE);
#endif

#ifdef DEFOG_HEATER_ENABLE
#pragma message("defog enabled")
R51::Defog defog(DEFOG_HEATER_PIN, DEFOG_HEATER_MS);
#endif

#ifdef STEERING_KEYPAD_ENABLE
#pragma message("steering keypad enabled")
R51::SteeringKeypad steering_keypad(STEERING_PIN_A, STEERING_PIN_B);
#endif

#ifdef BLUETOOTH_ENABLE
R51::BLE ble(BLUETOOTH_SPI_CS_PIN, BLUETOOTH_SPI_IRQ_PIN);
R51::BLENode ble_node(&ble);

void onBluetoothConnect() {
    ble_node.onConnect();
}

void onBluetoothDisconnect() {
    ble_node.onDisconnect();
}
#endif

#ifdef REALDASH_ENABLE
#pragma message("realdash enabled")
Canny::RealDash realdash_connection(&REALDASH_SERIAL);
RealDashAdapter realdash(&realdash_connection, REALDASH_FRAME_ID,
        REALDASH_HB_ID, REALDASH_HB_MS);
#endif

Bus<Message>* bus;
Node<Message>* nodes[] = {
    &can,
    &climate,
    &settings,
    &ipdm,
    &tires,
#if defined(CONSOLE_ENABLE) && defined(SERIAL_DEVICE)
    &console,
#endif
#ifdef DEFOG_HEATER_ENABLE
    &defog,
#endif
#ifdef STEERING_KEYPAD_ENABLE
    &steering_keypad,
#endif
#ifdef BLUETOOTH_ENABLE
    &ble_node,
#endif
#ifdef REALDASH_ENABLE
    &realdash,
#endif
};

void setup_serial() {
#ifdef SERIAL_DEVICE
    SERIAL_DEVICE.begin(SERIAL_BAUDRATE);
    if (SERIAL_WAIT) {
        while(!SERIAL_DEVICE) {
            delay(100);
        }
    }
#endif
}

void setup_can() {
    DEBUG_MSG("setup: connecting to can bus");
    while (!CAN.begin(Canny::CAN20_250K)) {
        DEBUG_MSG("setup: failed to init can bus");
        delay(200);
    }
}

void setup_bluetooth() {
#ifdef BLUETOOTH_ENABLE
    DEBUG_MSG("setup: initializing bluetooth");
    while (!ble.begin()) {
        DEBUG_MSG("setup: failed to init bluetooth");
        delay(500);
    }
    ble.setOnConnect(onBluetoothConnect);
    ble.setOnDisconnect(onBluetoothDisconnect);
#endif
}

void setup_bus() {
    DEBUG_MSG("setup: initializing bus");
    bus = new Bus<Message>(nodes, sizeof(nodes)/sizeof(nodes[0]));
}

void setup() {
    setup_serial();
    setup_can();
    setup_bluetooth();
    setup_bus();
    DEBUG_MSG("setup: ecu started");
}

void loop() {
    bus->loop();
#ifdef BLUETOOTH_ENABLE
    ble.update(BLUETOOTH_UPDATE_MS);
#endif
}
