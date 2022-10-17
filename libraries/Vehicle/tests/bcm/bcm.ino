#include <AUnit.h>
#include <Arduino.h>
#include <Faker.h>
#include <Test.h>
#include <Vehicle.h>

namespace R51 {

using namespace aunit;
using ::Faker::FakeClock;
using ::Faker::FakeGPIO;

test(DefrostTest, Trigger) {
    FakeYield yield;
    FakeClock clock;
    FakeGPIO gpio;
    Defrost defrost(1, 200, &clock, &gpio);
    Event event((uint8_t)SubSystem::BCM, (uint8_t)BCMEvent::TOGGLE_DEFROST_CMD);

    // Ensure default state is off.
    defrost.emit(yield);
    assertSize(yield, 0);
    assertEqual(gpio.digitalRead(1), 0);

    // Toggle the defrost heater.
    clock.set(1);
    defrost.handle(event, yield);
    defrost.emit(yield);
    assertSize(yield, 0);
    assertEqual(gpio.digitalRead(1), 1);

    // Ensure state is on after only part of the time elapses.
    clock.set(200);
    defrost.emit(yield);
    assertSize(yield, 0);
    assertEqual(gpio.digitalRead(1), 1);

    // Ensure state is off after the entire time elapses.
    clock.set(201);
    defrost.emit(yield);
    assertSize(yield, 0);
    assertEqual(gpio.digitalRead(1), 0);

    // State stays the same.
    clock.set(1000);
    defrost.emit(yield);
    assertSize(yield, 0);
    assertEqual(gpio.digitalRead(1), 0);

    // Trigger it again.
    clock.set(1001);
    defrost.handle(event, yield);
    defrost.emit(yield);
    assertSize(yield, 0);
    assertEqual(gpio.digitalRead(1), 1);

    clock.set(1200);
    defrost.emit(yield);
    assertSize(yield, 0);
    assertEqual(gpio.digitalRead(1), 1);

    clock.set(1201);
    defrost.emit(yield);
    assertSize(yield, 0);
    assertEqual(gpio.digitalRead(1), 0);
}

}  // namespace R51

// Test boilerplate.
void setup() {
#ifdef ARDUINO
    delay(1000);
#endif
    SERIAL_PORT_MONITOR.begin(115200);
    while(!SERIAL_PORT_MONITOR);
}

void loop() {
    aunit::TestRunner::run();
    delay(1);
}
