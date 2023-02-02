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
#if defined MULTICORE
// Core Synchronization
SyncWait sync;

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
#else
Node<Message>* io_nodes[] = {
    &ble_monitor,
    &realdash_gw,
    &console,
};
Bus<Message> io_bus(io_nodes, sizeof(io_nodes)/sizeof(io_nodes[0]));
#endif

void setup_spi() {
    DEBUG_MSG("setup: SPI");
    pinMode(MCP2515_CS_PIN, OUTPUT);
    digitalWrite(MCP2515_CS_PIN, LOW);
    pinMode(MCP2518_CS_PIN, OUTPUT);
    digitalWrite(MCP2518_CS_PIN, LOW);
    pinMode(BLUETOOTH_SPI_CS_PIN, OUTPUT);
    digitalWrite(BLUETOOTH_SPI_CS_PIN, LOW);
    SPI.begin();
}

void setup_bluetooth() {
    DEBUG_MSG("setup: Bluetooth");
    while (!ble_conn.begin()) {
        DEBUG_MSG("setup: Bluetooth failed");
        delay(500);
    }
    ble_conn.setName(BLUETOOTH_DEVICE_NAME);
    ble_conn.setOnConnect(onBluetoothConnect, nullptr);
    ble_conn.setOnDisconnect(onBluetoothDisconnect, nullptr);
}

void setup() {
    // Pico Core starts serial before setup so no need for begin().
    if (SERIAL_WAIT) {
        while(!SERIAL_DEVICE) {
            delay(100);
        }
    }

    setup_spi();
    setup_bluetooth();

#if defined(MULTICORE)
    sync.wait();
#endif
    DEBUG_MSG("setup: ECU running");
    io_bus.init();
    DEBUG_MSG("setup: bus init complete");
}

void loop() {
    io_bus.loop();
    ble_conn.update(BLUETOOTH_UPDATE_MS);
    delay(10);
}

#if defined(MULTICORE)
void setup1() {
    // Pico Core starts serial before setup so no need for begin().
    if (SERIAL_WAIT) {
        while(!SERIAL_DEVICE) {
            delay(100);
        }
    }

    sync.wait();
    proc_bus.init();
}

void loop1() {
    proc_bus.loop();
    delay(10);
}
#endif
