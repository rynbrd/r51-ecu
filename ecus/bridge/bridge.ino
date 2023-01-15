#include "Config.h"
#include "Debug.h"

#include <Arduino.h>
#include <Bluetooth.h>
#include <Canny.h>
#include <Canny/RealDash.h>
#include <Caster.h>
#include <Core.h>
#include <Platform.h>
#include <Vehicle.h>
#include "CAN.h"
#include "Debug.h"
#include "J1939.h"
#include "Pipe.h"

#ifndef ARDUINO_RASPBERRY_PI_PICO
#error "Target platform is not Longan RP2040."
#endif

extern "C" {
    #include <hardware/watchdog.h>
};

using namespace ::R51;
using ::Caster::Bus;
using ::Caster::Node;

// debug console
#if defined(CONSOLE_ENABLE)
#include <Console.h>
R51::ConsoleNode console(&SERIAL_DEVICE);
#endif

// vehicle CAN connection
CANConnection can_conn;
CANGateway can_gw(&can_conn);

// control system J1939 connection
#if defined(J1939_ENABLE)
J1939Connection j1939_conn;
J1939Gateway j1939_gw(&j1939_conn, J1939_ADDRESS, J1939_NAME, J1939_PROMISCUOUS);
J1939Adapter j1939_adapter;
#endif

// RealDash over Bluetooth
#if defined(BLUETOOTH_ENABLE)
BLE ble_conn(BLUETOOTH_SPI_CS_PIN, BLUETOOTH_SPI_IRQ_PIN);
BLENode ble_monitor(&ble_conn);

Canny::RealDash<Canny::CAN20Frame> realdash_serial(&ble_conn);
RealDashGateway realdash(&realdash_serial, REALDASH_FRAME_ID,
        REALDASH_HB_ID, REALDASH_HB_MS);

void onBluetoothConnect(void*) {
    ble_monitor.onConnect();
}

void onBluetoothDisconnect(void*) {
    ble_monitor.onDisconnect();
}
#endif

// vehicle management
PlatformConfigStore config;
Climate climate;
Settings settings;
IPDM ipdm;
Illum illum;
TirePressure tire_pressure(&config);
#if defined(DEFROST_HEATER_ENABLE)
Defrost defrost(DEFROST_HEATER_PIN, DEFROST_HEATER_MS);
#endif
#if defined(STEERING_KEYPAD_ENABLE)
SteeringKeypad steering_keypad(STEERING_KEYPAD_ID, STEERING_PIN_A, STEERING_PIN_B);
#endif

// Create internal bus.
FilteredPipe pipe;

Node<Message>* io_nodes[] = {
    pipe.left(),
    &can_gw,
#if defined(J1939_ENABLE)
    &j1939_gw,
#endif
#if defined(STEERING_KEYPAD_ENABLE)
    &steering_keypad,
#endif
};
Bus<Message> io_bus(io_nodes, sizeof(io_nodes)/sizeof(io_nodes[0]));

Node<Message>* proc_nodes[] = {
    &climate,
    &settings,
    &ipdm,
    &illum,
    &tire_pressure,
#if defined(DEBUG_ENABLE)
    &console,
#endif
#if defined(J1939_ENABLE)
    &j1939_adapter,
#endif
#if defined(BLUETOOTH_ENABLE)
    &ble_monitor,
    &realdash,
#endif
#if defined(DEFROST_HEATER_ENABLE)
    &defrost,
#endif
};
Bus<Message> proc_bus(proc_nodes, sizeof(proc_nodes)/sizeof(proc_nodes[0]));

void setup_serial() {
#if defined(DEBUG_ENABLE) || defined(CONSOLE_ENABLE)
    // RP2040 already call Serial.begin(115200) so we only ensure serial is
    // online before starting the cores.
    if (SERIAL_WAIT) {
        while(!SERIAL_DEVICE) {
            delay(100);
        }
    }
#endif
}

void setup_watchdog()  {
#if !defined(DEBUG_ENABLE)
    watchdog_enable(500, 1);
#endif
}

void setup_can() {
    DEBUG_MSG("setup: connecting to CAN");
    while (!can_conn.begin()) {
        DEBUG_MSG("setup: failed to connect to CAN");
        delay(500);
    }
}

void setup_j1939() {
#if defined(J1939_ENABLE)
    DEBUG_MSG("setup: connecting to J1939");
    while (!j1939_conn.begin()) {
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
#if defined(BLUETOOTH_DEVICE_NAME)
    ble_conn.setName(BLUETOOTH_DEVICE_NAME);
#endif
    ble_conn.setOnConnect(onBluetoothConnect, nullptr);
    ble_conn.setOnDisconnect(onBluetoothDisconnect, nullptr);
#endif
}

void setup_defrost() {
    DEBUG_MSG("setup: initializing defrost GPIO");
    defrost.begin();
}

void setup_steering() {
    DEBUG_MSG("setup: initializing steering keypad GPIO");
    steering_keypad.begin();
}

void setup() {
    setup_serial();
    DEBUG_MSG("setup: core0 initializing");
    setup_watchdog();
    setup_can();
    setup_j1939();
    setup_ble();
    DEBUG_MSG("setup: core0 online");
    setup_defrost();
    setup_steering();
    io_bus.init();
}

void setup1() {
    setup_serial();
    DEBUG_MSG("setup: core1 online");
    proc_bus.init();
}

void loop() {
    io_bus .loop();
#if defined(BLUETOOTH_ENABLE)
    // BLE's update method needs to be called to enable connectivity callbacks
    ble_conn.update(BLUETOOTH_UPDATE_MS);
#endif
#if defined(DEBUG_ENABLE)
    delay(10);
#else
    watchdog_update();
#endif
}

// Processing main loop.
void loop1() {
    proc_bus.loop();
#if defined(DEBUG_ENABLE)
    delay(10);
#endif
}
