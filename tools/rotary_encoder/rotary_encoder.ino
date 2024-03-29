#include "Config.h"
#include "Debug.h"

#include <Arduino.h>
#include <Caster.h>
#include <Console.h>
#include <Core.h>
#include <RotaryEncoder.h>

using namespace ::R51;
using ::Caster::Bus;
using ::Caster::Node;

// Serial Console
ConsoleNode console(&SERIAL_DEVICE, false);

// Rotary Encoder Hardware
RotaryEncoder rotary_encoder0(&I2C_DEVICE, ROTARY_ENCODER_IRQ_PIN);
RotaryEncoder rotary_encoder1(&I2C_DEVICE, ROTARY_ENCODER_IRQ_PIN);
RotaryEncoder* rotary_encoders[] = {
    &rotary_encoder0,
    &rotary_encoder1,
};
RotaryEncoderGroup rotary_encoder_group(ROTARY_ENCODER_ID, rotary_encoders,
        sizeof(rotary_encoders)/sizeof(rotary_encoders[0]));

// Internal Bus
Node<Message>* nodes[] = {
    &console,
    &rotary_encoder_group,
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

    DEBUG_MSG("setup: configuring I2C");
#if defined(I2C_SDA_PIN) && defined(I2C_SCL_PIN)
    I2C_DEVICE.setSDA(I2C_SDA_PIN);
    I2C_DEVICE.setSCL(I2C_SCL_PIN);
#endif

    DEBUG_MSG("setup: configuring rotary encoders");
    if (!rotary_encoder0.begin(ROTARY_ENCODER_ADDR0)) {
        DEBUG_MSG("setup: failed to start encoder 0");
        while (true) { delay(100); }
    }
    if (!rotary_encoder1.begin(ROTARY_ENCODER_ADDR1)) {
        DEBUG_MSG("setup: failed to start encoder 1");
        while (true) { delay(100); }
    }

    DEBUG_MSG("setup: initializing internal bus");
    bus.init();

    DEBUG_MSG("setup: ECU started");
}

void loop() {
    bus.loop();
    delay(10);
}
