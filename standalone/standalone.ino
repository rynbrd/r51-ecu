#include "Config.h"
#include "Debug.h"

#include <Arduino.h>
#include <Blink.h>
#include <Caster.h>
#include <Common.h>
#include <Controls.h>
#include <RotaryEncoder.h>
#include <Vehicle.h>
#include "CAN.h"
#include "J1939.h"
#include "Pico.h"

using namespace ::R51;
using ::Caster::Bus;
using ::Caster::Node;

// Init console support if enabled. 
#if defined(cONSOLE_ENABLE)
R51::ConsoleNode console(&SERIAL_DEVICE);
#endif

PicoConfigStore config;

// TODO: Integrate RP2040 watchdog timer.
// TODO: Add BLE RealDash integration.

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
RotaryEncoder rotary_encoder0(&Wire);
RotaryEncoder rotary_encoder1(&Wire);
RotaryEncoder* rotary_encoders[] = {
    &rotary_encoder0,
    &rotary_encoder1,
};
RotaryEncoderGroup rotary_encoder_group(ROTARY_ENCODER_ID, rotary_encoders,
        sizeof(rotary_encoders)/sizeof(rotary_encoders[0]));

// Enable rotary encoder interrupts if is configured.
#if defined(ROTARY_ENCODER_INTR_PIN)
void rotaryEncoderISR() {
    rotary_encoder_group.interrupt(0xFF);
}

void attachInterrupts() {
    attachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_INTR_PIN), rotaryEncoderISR, FALLING);
}

void detachInterrupts() {
    detachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_INTR_PIN));
}
#endif

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
PicoFilteredPipe pipe;

Node<Message>* io_nodes[] = {
    pipe.left(),
    &can_gw,
    &j1939_gw,
    &steering_keypad,
    &rotary_encoder_group,
};
Bus<Message> io_bus(io_nodes, sizeof(io_nodes)/sizeof(io_nodes[0]));

Node<Message>* proc_nodes[] = {
    pipe.right(),
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

void setup_can() {
    DEBUG_MSG("setup: connecting to CAN");
    while (!CAN.begin(VEHICLE_CAN_MODE)) {
        DEBUG_MSG("setup: failed to connect to CAN");
        delay(500);
    }
}

void setup_j1939() {
    DEBUG_MSG("setup: connecting to J1939");
    while (!J1939.begin(J1939_CAN_MODE)) {
        DEBUG_MSG("setup: failed to connect to J1939");
        delay(500);
    }
}

void setup_hmi() {
    DEBUG_MSG("setup: configuring HMI");
    HMI_DEVICE.begin(HMI_BAUDRATE);
}

void setup_rotary_encoders() {
    DEBUG_MSG("setup: configuring rotary encoders");
    rotary_encoder0.begin(ROTARY_ENCODER_ADDR0);
    rotary_encoder1.begin(ROTARY_ENCODER_ADDR1);
}

// I/O core setup.
void setup() {
    setup_serial();
    DEBUG_MSG("setup: core0 initializing");
    setup_can();
    setup_j1939();
    setup_rotary_encoders();
    DEBUG_MSG("setup: core0 online");
    io_bus.init();
}

void setup1() {
    setup_serial();
    DEBUG_MSG("setup: core1 initializing");
    setup_hmi();
    DEBUG_MSG("setup: core1 online");
    proc_bus.init();
}

void loop() {
    io_bus.loop();
#if defined(DEBUG_ENABLE)
    delay(10);
#endif
}

void loop1() {
    proc_bus.loop();
#if defined(DEBUG_ENABLE)
    delay(10);
#endif
}
