#ifndef __R51_TESTS_TEST_MOMENTARY_OUTPUT__
#define __R51_TESTS_TEST_MOMENTARY_OUTPUT__

#include <AUnit.h>
#include <Arduino.h>
#include <Faker.h>

#include "src/momentary_output.h"

using namespace aunit;


test(MomentaryOutputTest, TriggerHigh) {
    Faker::FakeClock clock;
    Faker::FakeGPIO gpio;

    MomentaryOutput output(16, 100, &clock, &gpio);
    assertEqual(gpio.pinMode(16), (uint32_t)OUTPUT);
    assertEqual(gpio.digitalRead(16), LOW);

    output.update();
    assertTrue(output.trigger());
    assertEqual(gpio.digitalRead(16), HIGH);

    clock.delay(1);
    output.update();
    assertFalse(output.trigger());
    assertEqual(gpio.digitalRead(16), HIGH);

    clock.delay(100);
    output.update();
    assertEqual(gpio.digitalRead(16), LOW);
    assertFalse(output.trigger());

    clock.delay(100);
    output.update();
    assertEqual(gpio.digitalRead(16), LOW);
    assertTrue(output.trigger());
    assertEqual(gpio.digitalRead(16), HIGH);
}

test(MomentaryOutputTest, TriggerLow) {
    Faker::FakeClock clock;
    Faker::FakeGPIO gpio;

    MomentaryOutput output(16, 100, -1, false, &clock, &gpio);
    assertEqual(gpio.pinMode(16), (uint32_t)OUTPUT);
    assertEqual(gpio.digitalRead(16), HIGH);

    output.update();
    assertTrue(output.trigger());
    assertEqual(gpio.digitalRead(16), LOW);

    clock.delay(1);
    output.update();
    assertFalse(output.trigger());
    assertEqual(gpio.digitalRead(16), LOW);

    clock.delay(100);
    output.update();
    assertEqual(gpio.digitalRead(16), HIGH);
    assertFalse(output.trigger());

    clock.delay(100);
    output.update();
    assertEqual(gpio.digitalRead(16), HIGH);
    assertTrue(output.trigger());
    assertEqual(gpio.digitalRead(16), LOW);
}

test(MomentaryOutputTest, ShortCooldown) {
    Faker::FakeClock clock;
    Faker::FakeGPIO gpio;

    MomentaryOutput output(16, 200, 10, true, &clock, &gpio);
    assertEqual(gpio.pinMode(16), (uint32_t)OUTPUT);
    assertEqual(gpio.digitalRead(16), LOW);

    output.update();
    assertTrue(output.trigger());
    assertEqual(gpio.digitalRead(16), HIGH);

    clock.delay(1);
    output.update();
    assertFalse(output.trigger());
    assertEqual(gpio.digitalRead(16), HIGH);

    clock.delay(200);
    output.update();
    assertEqual(gpio.digitalRead(16), LOW);
    assertFalse(output.trigger());

    clock.delay(10);
    output.update();
    assertEqual(gpio.digitalRead(16), LOW);
    assertTrue(output.trigger());
    assertEqual(gpio.digitalRead(16), HIGH);
}

#endif  // __R51_TESTS_TEST_MOMENTARY_OUTPUT__
