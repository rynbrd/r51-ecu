#include "Config.h"
#include "Debug.h"

#include <Arduino.h>
#include <Bluetooth.h>
#include <Canny.h>
#include <Canny/Detect.h>
#include <Canny/RealDash.h>
#include <Caster.h>
#include <Console.h>
#include <Vehicle.h>
#include "CAN.h"
#include "Debug.h"
#include "J1939.h"
#include "Pico.h"
#include "RealDash.h"

using ::Caster::Bus;
using ::Caster::Node;
using ::R51::BLE;
using ::R51::BLENode;
using ::R51::CANGateway;
using ::R51::Climate;
using ::R51::Defrost;
using ::R51::FilteredCAN;
using ::R51::IPDM;
using ::R51::J1939Adapter;
using ::R51::J1939Connection;
using ::R51::J1939Gateway;
using ::R51::Message;
using ::R51::PicoConfigStore;
using ::R51::RealDashAdapter;
using ::R51::Settings;
using ::R51::SteeringKeypad;
using ::R51::TirePressure;

// debug console
#if defined(DEBUG_ENABLE)
R51::ConsoleNode console(&SERIAL_DEVICE);
#endif

// vehicle CAN connection
FilteredCAN can_conn(&CAN);
CANGateway can_gw(&can_conn);

// control system J1939 connection
#if defined(J1939_ENABLE)
J1939Connection j1939_conn(&CAN1);
J1939Gateway j1939_gateway(&j1939_conn, J1939_ADDRESS, J1939_NAME, J1939_PROMISCUOUS);
J1939Adapter j1939_adapter;
#endif

// RealDash over Bluetooth
#if defined(BLUETOOTH_ENABLE)
BLE ble_conn(BLUETOOTH_SPI_CS_PIN, BLUETOOTH_SPI_IRQ_PIN);
BLENode ble_monitor(&ble_conn);

Canny::RealDash realdash_serial(&ble_conn);
RealDashAdapter realdash(&realdash_serial, REALDASH_FRAME_ID,
        REALDASH_HB_ID, REALDASH_HB_MS);

void onBluetoothConnect(void*) {
    ble_monitor.onConnect();
}

void onBluetoothDisconnect(void*) {
    ble_monitor.onDisconnect();
}
#endif

// vehicle management
PicoConfigStore config;
Climate climate;
Settings settings;
IPDM ipdm;
TirePressure tires_node;
#if defined(DEFROST_HEATER_ENABLE)
Defrost defrost(DEFROST_HEATER_PIN, DEFROST_HEATER_MS);
#endif
#if defined(STEERING_KEYPAD_ENABLE)
SteeringKeypad steering_keypad(STEERING_KEYPAD_ID, STEERING_PIN_A, STEERING_PIN_B);
#endif

Node<Message>* nodes[] = {
#if defined(DEBUG_ENABLE)
    &console,
#endif
    &can_gw,
#if defined(J1939_ENABLE)
    &j1939_gateway,
    &j1939_adapter,
#endif
#if defined(BLUETOOTH_ENABLE)
    &ble_monitor,
    &realdash,
    &climate,
    &settings,
#endif
    &ipdm,
#if defined(DEFOG_HEATER_ENABLE)
    &defrost,
#endif
#if defined(STEERING_KEYPAD_ENABLE)
    &steering_keypad,
#endif
};
Bus<Message> bus(nodes, sizeof(nodes)/sizeof(nodes[0]));

void setup_serial() {
    SERIAL_DEVICE.begin(SERIAL_BAUDRATE);
    if (SERIAL_WAIT) {
        while(!SERIAL_DEVICE) {
            delay(100);
        }
    }
    DEBUG_MSG("setup: ECU booting");
}

void setup_can() {
    DEBUG_MSG("setup: connecting to CAN");
    while (!CAN.begin(VEHICLE_CAN_MODE)) {
        DEBUG_MSG("setup: failed to connect to CAN");
        delay(500);
    }
}

void setup_j1939() {
#if defined(J1939_ENABLE)
    DEBUG_MSG("setup: connecting to J1939");
    while (!CAN1.begin(J1939_CAN_MODE)) {
        DEBUG_MSG("setup: failed to connect to J1939");
        delay(500);
    }
#endif
}

void setup_ble() {
#if defined(BLUETOOTH_ENABLE)
    DEBUG_MSG("setup: initializing bluetooth");
    while (!ble_conn.begin()) {
        DEBUG_MSG("setup: failed to init bluetooth");
        delay(500);
    }
    ble_conn.setOnConnect(onBluetoothConnect, nullptr);
    ble_conn.setOnDisconnect(onBluetoothDisconnect, nullptr);
#endif
}

void setup() {
    setup_serial();
    setup_can();
    setup_j1939();
    setup_ble();
    DEBUG_MSG("setup: ECU running");
    bus.init();
}

void loop() {
    bus.loop();
#if defined(BLUETOOTH_ENABLE)
    // BLE's update method needs to be called to enable connectivity callbacks
    ble_conn.update(BLUETOOTH_UPDATE_MS);
#endif
#if defined(DEBUG_ENABLE)
    delay(10);
#endif
}
