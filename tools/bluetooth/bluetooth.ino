#include "Config.h"
#include "Debug.h"

#include <Arduino.h>
#include <Bluetooth.h>
#include <Canny.h>
#include <Canny/RealDash.h>
#include <Caster.h>
#include <Console.h>
#include <Core.h>

using namespace ::R51;
using ::Caster::Bus;
using ::Caster::Node;

// Serial Console
ConsoleNode console(&SERIAL_DEVICE, false);

// Bluetooth Hardware Integration
BLE ble_conn(BLUETOOTH_SPI_CS_PIN, BLUETOOTH_SPI_IRQ_PIN);
BLENode ble_monitor(&ble_conn);

void onBluetoothConnect(void*) {
    ble_monitor.onConnect();
}

void onBluetoothDisconnect(void*) {
    ble_monitor.onDisconnect();
}

// Realdash Serial Over Bluetooth
Canny::RealDash<Canny::CAN20Frame> realdash_conn(&ble_conn);
RealDashGateway realdash_gw(&realdash_conn, REALDASH_FRAME_ID,
        REALDASH_HB_ID, REALDASH_HB_MS);

// Internal Bus
Node<Message>* nodes[] = {
    &console,
    &ble_monitor,
    &realdash_gw,
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

    DEBUG_MSG("setup: initializing bluetooth");
    while (!ble_conn.begin()) {
        DEBUG_MSG("setup: failed to init bluetooth");
        delay(500);
    }
    ble_conn.setName(BLUETOOTH_DEVICE_NAME);
    ble_conn.setOnConnect(onBluetoothConnect, nullptr);
    ble_conn.setOnDisconnect(onBluetoothDisconnect, nullptr);

    DEBUG_MSG("setup: initializing internal bus");
    bus.init();

    DEBUG_MSG("setup: ECU started");
}

void loop() {
    bus.loop();
    ble_conn.update(BLUETOOTH_UPDATE_MS);
    delay(10);
}
