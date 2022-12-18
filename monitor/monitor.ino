#include "Config.h"
#include "Debug.h"

#include <Arduino.h>
#include <Canny.h>
#include <Canny/Detect.h>
#include <Caster.h>
#include <Core.h>
#include <Console.h>

using ::Canny::CAN20Frame;
using ::Canny::J1939Message;
using ::Caster::Bus;
using ::Caster::Node;
using ::R51::CANGateway;
using ::R51::ConsoleNode;
using ::R51::Message;

// Uncomment to read frames as J1939 messages.
#define J1939_ENABLE

class LoggingCANGateway : public CANGateway {
    public:
        LoggingCANGateway(Canny::Connection* can) : CANGateway(can) {}

        void onReadError(Canny::Error err) {
            DEBUG_MSG_VAL("can: read error: ", err);
        }

        void onWriteError(Canny::Error err, const CAN20Frame& frame) {
            DEBUG_MSG_VAL("can: write error: ", err);
            DEBUG_MSG_VAL("can: dropped frame: ", frame);
        }
};

class LoggingJ1939Gateway : public J1939Gateway {
    public:
        LoggingJ1939Gateway(Canny::Connection* can) : J1939Gateway(can) {}

        void onReadError(Canny::Error err) {
            DEBUG_MSG_VAL("j1939: read error: ", err);
        }

        void onWriteError(Canny::Error err, const J1939Message& msg) {
            DEBUG_MSG_VAL("j1939: write error: ", err);
            DEBUG_MSG_VAL("j1939: dropped frame: ", msg);
        }
};

#if defined(J1939_ENABLE)
LoggingJ1939Gateway can(&CAN);
#else
LoggingCANGateway can(&CAN);
#endif

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
