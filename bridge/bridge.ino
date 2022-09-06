#include "Config.h"
#include "Debug.h"

#include <Arduino.h>

#include <Canny.h>
#include <Canny/Detect.h>
#include "CAN.h"
#include "Core0.h"

R51::VehicleConnection vehicle_can(&CAN);
R51::Core0 core0(&vehicle_can);

void setup_serial() {
    SERIAL_DEVICE.begin(SERIAL_BAUDRATE);
    if (SERIAL_WAIT) {
        while(!SERIAL_DEVICE) {
            delay(100);
        }
    }
    DEBUG_MSG("setup: ecu booting");
}

void setup_can() {
    DEBUG_MSG("setup: connecting to can");
    while (!CAN.begin(VEHICLE_CAN_MODE)) {
        DEBUG_MSG("setup: failed to connect to can");
        delay(500);
    }
}

void setup() {
    setup_serial();
    setup_can();

    core0.setup();
    DEBUG_MSG("setup: ecu running");
}

void loop() {
    core0.loop();
}
