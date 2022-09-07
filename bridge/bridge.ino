#include "Config.h"
#include "Debug.h"

#include <Arduino.h>

#include <Canny.h>
#include <Canny/Detect.h>
#include "CAN.h"
#include "Core0.h"

#if defined(J1939_ENABLE) && defined(RASPBERRYPI_PICO)
R51::Core0 core0(&CAN, &CAN1, J1939_ADDRESS);
#else
R51::Core0 core0(&CAN);
#endif

void setup_serial() {
    SERIAL_DEVICE.begin(SERIAL_BAUDRATE);
    if (SERIAL_WAIT) {
        while(!SERIAL_DEVICE) {
            delay(100);
        }
    }
    DEBUG_MSG("setup: ECU booting");
}

void setup_can() {
    DEBUG_MSG("setup: connecting to CAN");
    while (!CAN.begin(VEHICLE_CAN_MODE)) {
        DEBUG_MSG("setup: failed to connect to CAN");
        delay(500);
    }
}

void setup_j1939() {
#if defined(J1939_ENABLE) && defined(RASPBERRYPI_PICO)
    DEBUG_MSG("setup: connecting to J1939");
    while (!CAN1.begin(J1939_CAN_MODE)) {
        DEBUG_MSG("setup: failed to connect to J1939");
        delay(500);
    }
#endif
}

void setup() {
    setup_serial();
    setup_can();
    setup_j1939();

    core0.setup();
    DEBUG_MSG("setup: ECU running");
}

void loop() {
    core0.loop();
}
