#include "Config.h"

#include <Arduino.h>

#include <Canny.h>
#include <Canny/Detect.h>
#include "Core0.h"

R51::Core0 core0(&CAN);

void setup() {
    SERIAL_DEVICE.begin(SERIAL_BAUDRATE);
    if (SERIAL_WAIT) {
        while(!SERIAL_DEVICE) {
            delay(100);
        }
    }

    core0.setup();
    DEBUG_MSG("setup: ecu started");
}

void loop() {
    core0.loop();
}
