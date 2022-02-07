#ifndef __R51_TESTS_STEERING__
#define __R51_TESTS_STEERING__

#include <AUnit.h>

#include "clock.h"
#include "gpio.h"
#include "src/bus.h"
#include "src/config.h"
#include "src/steering.h"

using namespace aunit;


class SteeringKeypadTest : public TestOnce {
    public:
        static constexpr const int values[] = STEERING_SWITCH_VALUES;

        void assertButtonPress(uint32_t pin, uint32_t value, byte expect) {
            MockClock clock;
            MockGPIO gpio;

            Frame released = {
                .id = 0x5800,
                .len = 8,
                .data = {
                    0x00, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00,
                },
            };
            Frame pressed = {
                .id = 0x5800,
                .len = 8,
                .data = {
                    expect, 0x00, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00,
                },
            };
            Frame actual;

            // initialize keypad
            SteeringKeypad keypad(&clock, &gpio);
            gpio.analogWrite(STEERING_SWITCH_A_PIN, 1023);
            gpio.analogWrite(STEERING_SWITCH_B_PIN, 1023);

            // receive heartbeat to ensure button not pressed
            clock.delay(501);
            assertTrue(keypad.receive(&actual));
            assertTrue(framesEqual(&actual, &released));

            // set analog value, check button, wait for debounce, check button again
            gpio.analogWrite(pin, value);
            assertFalse(keypad.receive(&actual));
            clock.delay(100);
            assertTrue(keypad.receive(&actual));
            assertTrue(framesEqual(&actual, &pressed));

            // release the button and advance time past the debounce
            gpio.analogWrite(pin, 1023);
            assertFalse(keypad.receive(&actual));
            clock.delay(100);
            assertTrue(keypad.receive(&actual));
            assertTrue(framesEqual(&actual, &released));
        }
};

testF(SteeringKeypadTest, Power) {
    assertButtonPress(STEERING_SWITCH_A_PIN, values[0], 0x01);
}

testF(SteeringKeypadTest, SeekDown) {
    assertButtonPress(STEERING_SWITCH_A_PIN, values[1], 0x20);
}

testF(SteeringKeypadTest, VolumeDown) {
    assertButtonPress(STEERING_SWITCH_A_PIN, values[2], 0x08);
}

testF(SteeringKeypadTest, Mode) {
    assertButtonPress(STEERING_SWITCH_B_PIN, values[0], 0x02);
}

testF(SteeringKeypadTest, SeekUp) {
    assertButtonPress(STEERING_SWITCH_B_PIN, values[1], 0x10);
}

testF(SteeringKeypadTest, VolumeUp) {
    assertButtonPress(STEERING_SWITCH_B_PIN, values[2], 0x04);
}

test(SteeringKeypad, Heartbeat) {
    MockClock clock;
    MockGPIO gpio;

    Frame expect = {
        .id = 0x5800,
        .len = 8,
        .data = {
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
        },
    };
    Frame actual;

    SteeringKeypad keypad(&clock, &gpio);
    gpio.analogWrite(STEERING_SWITCH_A_PIN, 1023);
    gpio.analogWrite(STEERING_SWITCH_B_PIN, 1023);

    clock.delay(501);
    assertTrue(keypad.receive(&actual));
    assertTrue(framesEqual(&actual, &expect));
    
    clock.delay(501);
    assertTrue(keypad.receive(&actual));
    assertTrue(framesEqual(&actual, &expect));
}

#endif  // __R51_TESTS_STEERING__
