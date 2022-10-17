#include "Config.h"
#include "Debug.h"

#include <Arduino.h>
#include <Blink.h>
#include <Canny.h>
#include <Canny/Detect.h>
#include <Controls.h>
#include <RotaryEncoder.h>
#include "J1939.h"

#if defined(DEBUG_ENABLE)
#include <Console.h>
#endif

using ::Canny::J1939Message;
using ::Caster::Bus;
using ::Caster::Node;
using ::R51::BlinkKeybox;
using ::R51::BlinkKeypad;
using ::R51::Fusion;
using ::R51::HMI;
using ::R51::J1939Connection;
using ::R51::J1939ControllerAdapter ;
using ::R51::J1939Gateway;
using ::R51::Message;
using ::R51::NavControls;
using ::R51::PowerControls;
using ::R51::RotaryEncoder;
using ::R51::RotaryEncoderGroup;
using ::R51::Scratch;
using ::R51::SteeringControls;

Scratch scratch;

//TODO: broadcast filtered events over J1939
J1939Connection j1939_conn(&CAN);
J1939Gateway j1939_gateway(&j1939_conn, J1939_ADDRESS, J1939_NAME, J1939_PROMISCUOUS);
J1939ControllerAdapter j1939_adapter;
HMI hmi(&HMI_DEVICE, &scratch);
Fusion fusion(&scratch);

RotaryEncoder rotary_encoder0(&Wire);
RotaryEncoder rotary_encoder1(&Wire);
RotaryEncoder* rotary_encoders[] = {
    &rotary_encoder0,
    &rotary_encoder1,
};
RotaryEncoderGroup rotary_encoder_group(ROTARY_ENCODER_ID, rotary_encoders,
        sizeof(rotary_encoders)/sizeof(rotary_encoders[0]));

BlinkKeypad blink_keypad(BLINK_KEYPAD_ID, BLINK_KEYPAD_ADDR, BLINK_KEYPAD_KEYS);
BlinkKeybox blink_keybox(BLINK_KEYBOX_ID, BLINK_KEYBOX_ADDR);

NavControls nav_controls(ROTARY_ENCODER_ID);
PowerControls power_controls(BLINK_KEYPAD_ID, BLINK_KEYBOX_ID);
SteeringControls steering_controls(STEERING_KEYPAD_ID);

#if defined(DEBUG_ENABLE)
R51::ConsoleNode console(&SERIAL_DEVICE, &scratch);
#endif

Node<Message>* nodes[] = {
#if defined(DEBUG_ENABLE)
    &console,
#endif
    &j1939_gateway,
    &j1939_adapter,
    &rotary_encoder_group,
    &blink_keypad,
    &blink_keybox,
    &hmi,
    &fusion,
    &nav_controls,
    &power_controls,
    &steering_controls,
};
Bus<Message> bus(nodes, sizeof(nodes)/sizeof(nodes[0]));

void rotaryEncoderISR() {
    rotary_encoder_group.interrupt(0xFF);
}

void attachInterrupts() {
    attachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_INTR_PIN), rotaryEncoderISR, FALLING);
}

void detachInterrupts() {
    detachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_INTR_PIN));
}

void setup_serial() {
#if defined(DEBUG_ENABLE)
    SERIAL_DEVICE.begin(SERIAL_BAUDRATE);
    if (SERIAL_WAIT) {
        while(!SERIAL_DEVICE) {
            delay(100);
        }
    }
    DEBUG_MSG("setup: ECU booting");
#endif
}

void setup_can() {
    DEBUG_MSG("setup: connecting to CAN");
    while (!CAN.begin(J1939_CAN_MODE)) {
        DEBUG_MSG("setup: failed to connect to CAN");
        delay(500);
    }
}

void setup_hmi() {
    DEBUG_MSG("setup: configuring HMI");
    HMI_DEVICE.begin(HMI_BAUDRATE);
}

void setup_keypads() {
    DEBUG_MSG("setup: configuring keypads");
    rotary_encoder0.begin(ROTARY_ENCODER_ADDR0);
    rotary_encoder1.begin(ROTARY_ENCODER_ADDR1);
}

void setup() {
    setup_serial();
    setup_can();
    setup_hmi();
    setup_keypads();
    DEBUG_MSG("setup: ECU started");
    bus.init();
}

void loop() {
    bus.loop();
#if defined(DEBUG_ENABLE)
    delay(10);
#endif
}
