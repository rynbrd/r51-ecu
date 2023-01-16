#include "Config.h"
#include "Debug.h"

#include <Arduino.h>
#include <Blink.h>
#include <Bluetooth.h>
#include <Canny.h>
#include <Canny/RealDash.h>
#include <Caster.h>
#include <Controls.h>
#include <Core.h>
#include <Platform.h>
#include <RotaryEncoder.h>
#include <Vehicle.h>
#include "CAN.h"
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

// Init core synchronization.
SyncWait sync;

// Init console support if enabled. 
#if defined(CONSOLE_ENABLE)
#include <Console.h>
ConsoleNode console(&SERIAL_DEVICE);
#endif

PlatformConfigStore config;

/**
 * Vehicle Integration
 * Create vehicle CAN bus connection and all R51 specific controller nodes.
 */

// Connect to the vehicle via CAN.
CANConnection can_conn;
CANGateway can_gw(&can_conn);

// Vehicle hardware integration modules. These integrate with the vehicle via GPIO.
Defrost defrost(DEFROST_HEATER_PIN, DEFROST_HEATER_MS);
SteeringKeypad steering_keypad(STEERING_KEYPAD_ID, STEERING_PIN_A, STEERING_PIN_B);

// Vehicle CAN controller modules. These translace between CAN frames and Events.
Climate climate;
Settings settings;
IPDM ipdm;
TirePressure tire_pressure(&config);
Illum illum;

/**
 * BLE and RealDash Integration
 * Support RealDash connectivity over Bluetooth.
 */
#if defined(BLUETOOTH_ENABLE)
BLE ble_conn(BLUETOOTH_SPI_CS_PIN, BLUETOOTH_SPI_IRQ_PIN);
BLENode ble_monitor(&ble_conn);
Canny::RealDash<Canny::CAN20Frame> realdash_conn(&ble_conn);
RealDashGateway realdash_gw(&realdash_conn, REALDASH_FRAME_ID,
        REALDASH_HB_ID, REALDASH_HB_MS);

void onBluetoothConnect(void*) {
    ble_monitor.onConnect();
}

void onBluetoothDisconnect(void*) {
    ble_monitor.onDisconnect();
}
#endif

/**
 * Aftermarket Integration
 * Create J1939 connection to integrate with third party/aftermarket equipment.
 */

// Create J1939 connection.
J1939Connection j1939_conn;
J1939Gateway j1939_gw(&j1939_conn, J1939_ADDRESS, J1939_NAME, J1939_PROMISCUOUS);

// J1939 hardware integrations.
Fusion fusion;
BlinkKeypad blink_keypad(BLINK_KEYPAD_ADDR, BLINK_KEYPAD_ID, BLINK_KEYPAD_KEYS);
BlinkKeybox blink_keybox(BLINK_KEYBOX_ADDR, BLINK_KEYBOX_ID);

/**
 * Human Interface Integration
 * Adds touch screen and rotary encoder support.
 */

// HMI screen node. This will live on the processing core since this hosts
// communication with the the visual user interface. We don't want this and the
// CAN modules to block each other.
HMI hmi(&HMI_DEVICE, ROTARY_ENCODER_ID, BLINK_KEYBOX_ID);

// Rotary encoder hardware. This node lives on the I/O core.
RotaryEncoder rotary_encoder0(&I2C_DEVICE, ROTARY_ENCODER_IRQ_PIN);
RotaryEncoder rotary_encoder1(&I2C_DEVICE, ROTARY_ENCODER_IRQ_PIN);
RotaryEncoder* rotary_encoders[] = {
    &rotary_encoder0,
    &rotary_encoder1,
};
RotaryEncoderGroup rotary_encoder_group(ROTARY_ENCODER_ID, rotary_encoders,
        sizeof(rotary_encoders)/sizeof(rotary_encoders[0]));

/**
 * Controller Nodes
 * These bridge communication between modules which do not communicate directly.
 */
NavControls nav_controls(ROTARY_ENCODER_ID);
PowerControls power_controls(BLINK_KEYPAD_ID, BLINK_KEYBOX_ID);
SteeringControls steering_controls(STEERING_KEYPAD_ID);

/**
 * Create Internal Bus
 */
FilteredPipe pipe;

Node<Message>* io_nodes[] = {
    pipe.left(),
    &can_gw,
    &j1939_gw,
    &steering_keypad,
    &rotary_encoder_group,
    &ble_monitor,
    &realdash_gw,
};
Bus<Message> io_bus(io_nodes, sizeof(io_nodes)/sizeof(io_nodes[0]));

Node<Message>* proc_nodes[] = {
    pipe.right(),
#if defined(CONSOLE_ENABLE)
    &console,
#endif
    &defrost,
    &climate,
    &settings,
    &ipdm,
    &tire_pressure,
    &illum,
    &fusion,
    &blink_keypad,
    &blink_keybox,
    &hmi,
    &nav_controls,
    &power_controls,
    &steering_controls,
};
Bus<Message> proc_bus(proc_nodes, sizeof(proc_nodes)/sizeof(proc_nodes[0]));

/**
 * Arduino Setup and Loop Functions
 */

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

void setup_spi() {
    DEBUG_MSG("setup: SPI");
    pinMode(MCP2515_CS_PIN, OUTPUT);
    digitalWrite(MCP2515_CS_PIN, LOW);
    pinMode(MCP2518_CS_PIN, OUTPUT);
    digitalWrite(MCP2518_CS_PIN, LOW);
#if defined(BLUETOOTH_ENABLE)
    pinMode(BLUETOOTH_SPI_CS_PIN, OUTPUT);
    digitalWrite(BLUETOOTH_SPI_CS_PIN, LOW);
#endif
    SPI.begin();
}

void setup_i2c() {
#if defined(I2C_SDA_PIN) && defined(I2C_SCL_PIN)
    DEBUG_MSG("setup: I2C");
    I2C_DEVICE.setSDA(I2C_SDA_PIN);
    I2C_DEVICE.setSCL(I2C_SCL_PIN);
#endif
}

void setup_can() {
    DEBUG_MSG("setup: CAN");
    while (!can_conn.begin()) {
        DEBUG_MSG("setup: CAN failed");
        delay(500);
    }
}

void setup_j1939() {
    DEBUG_MSG("setup: J1939");
    while (!j1939_conn.begin()) {
        DEBUG_MSG("setup: J1939 failed");
        delay(500);
    }
}

void setup_bluetooth() {
#if defined(BLUETOOTH_ENABLE)
    DEBUG_MSG("setup: Bluetooth");
    while (!ble_conn.begin()) {
        DEBUG_MSG("setup: Bluetooth failed to init");
        delay(500);
    }
#if defined(BLUETOOTH_DEVICE_NAME)
    ble_conn.setName(BLUETOOTH_DEVICE_NAME);
#endif
    ble_conn.setOnConnect(onBluetoothConnect, nullptr);
    ble_conn.setOnDisconnect(onBluetoothDisconnect, nullptr);
#endif
}

void setup_hmi() {
    DEBUG_MSG("setup: HMI");
    HMI_DEVICE.begin(HMI_BAUDRATE);
}

void setup_defrost() {
    DEBUG_MSG("setup: Defrost");
    defrost.begin();
}

void setup_steering() {
    DEBUG_MSG("setup: Steering Keypad");
    steering_keypad.begin();
}

void setup_rotary_encoders() {
    DEBUG_MSG("setup: Rotary Encoders");
    rotary_encoder0.begin(ROTARY_ENCODER_ADDR0);
    rotary_encoder1.begin(ROTARY_ENCODER_ADDR1);
}

// I/O core setup.
void setup() {
    setup_serial();
    setup_watchdog();
    setup_spi();
    setup_i2c();
    setup_can();
    setup_j1939();
    setup_bluetooth();
    setup_rotary_encoders();
    setup_defrost();
    setup_steering();
    sync.wait();
    DEBUG_MSG("setup: ECU running");
    io_bus.init();
}

// Processing core setup.
void setup1() {
    setup_serial();
    setup_hmi();
    sync.wait();
    proc_bus.init();
}

// I/O main loop.
void loop() {
    io_bus.loop();
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
