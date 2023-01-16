#include "Config.h"
#include "Debug.h"

#include <Arduino.h>
#include <Blink.h>
#include <Canny.h>
#include <Controls.h>
#include <Platform.h>
#include <RotaryEncoder.h>
#include "J1939.h"
#include "Pipe.h"

#ifndef ARDUINO_RASPBERRY_PI_PICO
#error "Target platform is not Longan RP2040."
#endif

extern "C" {
    #include <hardware/watchdog.h>
};

using namespace ::R51;
using ::Canny::J1939Message;
using ::Caster::Bus;
using ::Caster::Node;

// Init core synchronization.
SyncWait sync;

// Init console support if enabled. 
#if defined(CONSOLE_ENABLE)
#include <Console.h>
R51::ConsoleNode console(&SERIAL_DEVICE);
#endif

// Create J1939 connection.
J1939Connection j1939_conn;
J1939Gateway j1939_gw(&j1939_conn, J1939_ADDRESS, J1939_NAME, J1939_PROMISCUOUS);
J1939ControllerAdapter j1939_adapter;

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

// J1939 hardware integrations.
Fusion fusion;
BlinkKeypad blink_keypad(BLINK_KEYPAD_ADDR, BLINK_KEYPAD_ID, BLINK_KEYPAD_KEYS);
BlinkKeybox blink_keybox(BLINK_KEYBOX_ADDR, BLINK_KEYBOX_ID);

// Controller nodes.
NavControls nav_controls(ROTARY_ENCODER_ID);
PowerControls power_controls(BLINK_KEYPAD_ID, BLINK_KEYBOX_ID);
SteeringControls steering_controls(STEERING_KEYPAD_ID);

// Create internal bus.
FilteredPipe pipe;

Node<Message>* io_nodes[] = {
    pipe.left(),
    &j1939_gw,
    &rotary_encoder_group,
};
Bus<Message> io_bus(io_nodes, sizeof(io_nodes)/sizeof(io_nodes[0]));

Node<Message>* proc_nodes[] = {
    pipe.right(),
#if defined(DEBUG_ENABLE)
    &console,
#endif
    &j1939_adapter,
    &fusion,
    &blink_keypad,
    &blink_keybox,
    &hmi,
    &nav_controls,
    &power_controls,
    &steering_controls,
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

void setup_spi() {
    DEBUG_MSG("setup: SPI");
    pinMode(MCP2515_CS_PIN, OUTPUT);
    digitalWrite(MCP2515_CS_PIN, LOW);
    SPI.begin();
}

void setup_i2c() {
#if defined(I2C_SDA_PIN) && defined(I2C_SCL_PIN)
    DEBUG_MSG("setup: I2C");
    I2C_DEVICE.setSDA(I2C_SDA_PIN);
    I2C_DEVICE.setSCL(I2C_SCL_PIN);
#endif
}

void setup_j1939() {
    DEBUG_MSG("setup: J1939");
    while (!j1939_conn.begin()) {
        DEBUG_MSG("setup: J1939 failed");
        delay(500);
    }
}

void setup_hmi() {
    DEBUG_MSG("setup: HMI");
    HMI_DEVICE.begin(HMI_BAUDRATE);
}

void setup_rotary_encoders() {
    DEBUG_MSG("setup: Rotary Encoders");
    rotary_encoder0.begin(ROTARY_ENCODER_ADDR0);
    rotary_encoder1.begin(ROTARY_ENCODER_ADDR1);
}

void setup() {
    setup_serial();
    setup_spi();
    setup_watchdog();
    setup_i2c();
    setup_j1939();
    setup_rotary_encoders();
    sync.wait();
    DEBUG_MSG("setup: ECU running");
    io_bus.init();
}

void setup1() {
    setup_serial();
    setup_hmi();
    sync.wait();
    proc_bus.init();
}

// I/O main loop.
void loop() {
    io_bus.loop();
    watchdog_update();
}

// Processing main loop.
void loop1() {
    proc_bus.loop();
}
