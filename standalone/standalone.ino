#include <Arduino.h>
#include <Canny.h>
#include <Canny/Detect.h>
#include <Caster.h>

#include "src/can.h"
#include "src/climate.h"
#include "src/config.h"
#include "src/debug.h"
#include "src/events.h"
#include "src/realdash.h"
#include "src/serial.h"
#include "src/settings.h"
#include "src/steering.h"

using ::Canny::Frame;
using ::Caster::Bus;
using ::Caster::Node;

class ControllerCan : public CanNode {
    public:
        ControllerCan() : CanNode(&CAN) {}

        // Read all CAN frames.
        bool readFilter(const Frame& frame) const override {
            return (frame.id() & 0xFFFFFFFE) == 0x54A ||
                   (frame.id() & 0xFFFFFFFE) == 0x72E ||
                    frame.id() == 0x625;
        }

        // Only send climate and settings frames over CAN.
        bool writeFilter(const Frame& frame) const override {
            return (frame.id() & 0xFFFFFFFE) == 0x540 ||
                   (frame.id() & 0xFFFFFFFE) == 0x71E;
        }
};

class ControllerRealDash : public RealDash {
    public:
        // Only read dashboard control frames from RealDash.
        bool readFilter(const Frame& frame) const override {
            return frame.id() == 0x5401 ||
                   frame.id() == 0x5701;
        }

        // Only write dashboard state frames to RealDash.
        bool writeFilter(const Frame& frame) const override {
            return frame.id() == 0x5400 ||
                   frame.id() == 0x5700 ||
                   frame.id() == 0x5800;
        }
};

class ControllerSerialText : public SerialText {
    public:
        bool readFilter(const Frame&) const override { return true; }
        bool writeFilter(const Frame&) const override { return true; }
};

ControllerCan can;
Climate climate;
ControllerRealDash realdash;
Settings settings;
SteeringKeypad steering_keypad;
D(ControllerSerialText serial_text);

Bus<Frame>* bus;
Node<Frame>* nodes[] = {
    &can,
    &climate,
    &realdash,
    &settings,
    &steering_keypad,
#ifdef DEBUG
    &serial_text,
#endif
};

void setup_debug() {
    DEBUG_BEGIN();
    D(serial_text.begin(&DEBUG_SERIAL));
    #ifdef DEBUG_WAIT_FOR_SERIAL
    WAIT_FOR_SERIAL(DEBUG_SERIAL, 100, nullptr);
    #endif
}

void setup_realdash() {
    INFO_MSG("setup: connecting to realdash");
    REALDASH_SERIAL.begin(REALDASH_BAUDRATE);
    #ifdef REALDASH_WAIT_FOR_SERIAL
    WAIT_FOR_SERIAL(REALDASH_SERIAL, 1000, "setup: dash serial not ready");
    #endif
    realdash.begin(&REALDASH_SERIAL);
}

void setup_can() {
    INFO_MSG("setup: connecting to can bus");
    while (CAN.begin(Canny::CAN20_500K) != Canny::ERR_OK) {
        ERROR_MSG("setup: failed to init can bus");
        delay(500);
    }
}

void setup_bus() {
    INFO_MSG("setup: initializing bus");
    bus = new Bus(nodes, sizeof(nodes)/sizeof(nodes[0]));
}

void setup() {
    D(setup_debug());
    setup_realdash();
    setup_can();
    setup_bus();
    INFO_MSG("setup: ecu started");
}

void loop() {
    bus->loop();
}
