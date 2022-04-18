#ifndef __R51_TESTS_TEST_STEERING__
#define __R51_TESTS_TEST_STEERING__

#include <AUnit.h>
#include <Arduino.h>
#include <Canny.h>
#include <Faker.h>

#include "mock_yield.h"
#include "src/config.h"
#include "src/steering.h"
#include "testing.h"

using namespace aunit;
using ::Canny::Frame;


class SteeringKeypadTest : public TestOnce {
    public:
        static constexpr const int values[] = STEERING_SWITCH_VALUES;

        void assertButtonPress(uint32_t pin, uint32_t value, byte expect) {
            Faker::FakeClock clock;
            Faker::FakeGPIO gpio;
            MockYield yield(1);

            Frame released(0x5800, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
            Frame pressed(0x5800, 0, {expect, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

            // initialize keypad
            SteeringKeypad keypad(&clock, &gpio);
            gpio.analogWrite(STEERING_SWITCH_A_PIN, 1023);
            gpio.analogWrite(STEERING_SWITCH_B_PIN, 1023);

            // emit heartbeat to ensure button not pressed
            clock.delay(501);
            keypad.emit(yield.impl);
            assertEqual(yield.count(), 1);
            assertTrue(checkFrameEquals(yield.frames()[0], released));
            yield.reset();

            // set analog value, check button, wait for debounce, check button again
            gpio.analogWrite(pin, value);
            keypad.emit(yield.impl);
            assertEqual(yield.count(), 0);
            clock.delay(100);
            keypad.emit(yield.impl);
            assertEqual(yield.count(), 1);
            assertTrue(checkFrameEquals(yield.frames()[0], pressed));
            yield.reset();

            // release the button and advance time past the debounce
            gpio.analogWrite(pin, 1023);
            keypad.emit(yield.impl);
            assertEqual(yield.count(), 0);
            clock.delay(100);
            keypad.emit(yield.impl);
            assertEqual(yield.count(), 1);
            assertTrue(checkFrameEquals(yield.frames()[0], released));
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
    Faker::FakeClock clock;
    Faker::FakeGPIO gpio;
    MockYield yield(1);

    Frame expect(0x5800, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    SteeringKeypad keypad(&clock, &gpio);
    gpio.analogWrite(STEERING_SWITCH_A_PIN, 1023);
    gpio.analogWrite(STEERING_SWITCH_B_PIN, 1023);

    clock.delay(501);
    keypad.emit(yield.impl);
    assertEqual(yield.count(), 1);
    assertTrue(checkFrameEquals(yield.frames()[0], expect));
    yield.reset();
    
    clock.delay(501);
    keypad.emit(yield.impl);
    assertEqual(yield.count(), 1);
    assertTrue(checkFrameEquals(yield.frames()[0], expect));
}

#endif  // __R51_TESTS_TEST_STEERING__
