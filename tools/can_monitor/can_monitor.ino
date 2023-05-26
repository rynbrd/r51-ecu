#include "Config.h"

#include <Arduino.h>
#include <Canny.h>
#include <Canny/MCP2515.h>
#include <Caster.h>
#include <Console.h>
#include <SPI.h>
#include "CAN.h"

using namespace ::R51;
using ::Caster::Bus;
using ::Caster::Node;

CANConnection can_conn;
CANGateway can_gw(&can_conn);
ConsoleNode console(&SERIAL_DEVICE, false);

Node<Message>* nodes[] = {
    &can_gw,
    &console,
};
Bus<Message> bus(nodes, sizeof(nodes)/sizeof(nodes[0]));

void setup_spi() {
    pinMode(MCP2515_CS_PIN, OUTPUT);
    digitalWrite(MCP2515_CS_PIN, LOW);
    pinMode(MCP2518_CS_PIN, OUTPUT);
    digitalWrite(MCP2518_CS_PIN, LOW);
    SPI.begin();
}

void setup_can() {
    while (!can_conn.begin()) {
        Serial.println("setup: CAN failed");
        delay(1000);
    }
    Serial.println("setup: CAN");
}

void setup() {
    if (SERIAL_WAIT) {
        while(!SERIAL_DEVICE) {
            delay(100);
        }
    }
    setup_spi();
    setup_can();
    bus.init();
    Serial.println("setup: ECU Running");
}

void loop() {
    bus.loop();
}
