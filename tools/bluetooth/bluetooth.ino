#include "Config.h"
#include "Debug.h"

#include <Arduino.h>
#include <Bluetooth.h>
#include <Canny.h>
#include <Canny/RealDash.h>
#include <Caster.h>
#include <Console.h>
#include <Core.h>
#include <Platform.h>

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
Pipe pipe(IO_CORE_BUFFER_SIZE, PROC_CORE_BUFFER_SIZE);
Node<Message>* io_nodes[] = {
    pipe.left(),
    &ble_monitor,
    &realdash_gw,
};
Bus<Message> io_bus(io_nodes, sizeof(io_nodes)/sizeof(io_nodes[0]));

Node<Message>* proc_nodes[] = {
    pipe.right(),
    &console,
};
Bus<Message> proc_bus(proc_nodes, sizeof(proc_nodes)/sizeof(proc_nodes[0]));

void setup() {
    // Pico Core starts serial before setup so no need for begin().
    if (SERIAL_WAIT) {
        while(!SERIAL_DEVICE) {
            delay(100);
        }
    }
    DEBUG_MSG("setup: initializing IO core");

    DEBUG_MSG("setup: initializing bluetooth");
    while (!ble_conn.begin()) {
        DEBUG_MSG("setup: failed to init bluetooth");
        delay(500);
    }
    ble_conn.setName(BLUETOOTH_DEVICE_NAME);
    ble_conn.setOnConnect(onBluetoothConnect, nullptr);
    ble_conn.setOnDisconnect(onBluetoothDisconnect, nullptr);

    DEBUG_MSG("setup: initializing IO bus");
    io_bus.init();

    DEBUG_MSG("setup: IO core started");
}

void setup1() {
    // Pico Core starts serial before setup so no need for begin().
    if (SERIAL_WAIT) {
        while(!SERIAL_DEVICE) {
            delay(100);
        }
    }
    DEBUG_MSG("setup: initializing processing core");

    DEBUG_MSG("setup: initializing processing bus");
    proc_bus.init();

    DEBUG_MSG("setup: processing core started");
}

void loop() {
    io_bus.loop();
    ble_conn.update(BLUETOOTH_UPDATE_MS);
    delay(10);
}

void loop1() {
    proc_bus.loop();
    delay(10);
}
