#include "Config.h"
#include "Debug.h"

#include <Arduino.h>
#include <Canny.h>
#include <Canny/Detect.h>
#include <Caster.h>
#include <Common.h>
#include <Console.h>

using ::Canny::Frame;
using ::Caster::Bus;
using ::Caster::Node;
using ::R51::CANNode;
using ::R51::ConsoleNode;
using ::R51::Message;

class LoggingCANNode : public CANNode {
    public:
        LoggingCANNode(Canny::Connection* can) : CANNode(can) {}

        void onReadError(Canny::Error err) {
            DEBUG_MSG_VAL("can: read error: ", err);
        }

        void onWriteError(Canny::Error err, const Canny::Frame& frame) {
            DEBUG_MSG_VAL("can: write error: ", err);
            DEBUG_MSG_VAL("can: dropped frame: ", frame);
        }
};

LoggingCANNode can(&CAN);
ConsoleNode console(&SERIAL_DEVICE);

Bus<Message>* bus;
Node<Message>* nodes[] = {
    &can,
    &console,
};

void setup_serial() {
    SERIAL_DEVICE.begin(SERIAL_BAUDRATE);
    if (SERIAL_WAIT) {
        while (!SERIAL_DEVICE) {
            delay(100);
        }
    }
}

void setup_can() {
    DEBUG_MSG("setup: connecting to can bus");
    while (!CAN.begin(CAN_MODE)) {
        DEBUG_MSG("setup: failed to init can bus");
        delay(500);
    }
}

void setup_bus() {
    DEBUG_MSG("setup: initializing bus");
    bus = new Bus<Message>(nodes, sizeof(nodes)/sizeof(nodes[0]));
}

void setup() {
    setup_serial();
    setup_can();
    setup_bus();
    DEBUG_MSG("setup: ecu started");
}

void loop() {
    bus->loop();
}
