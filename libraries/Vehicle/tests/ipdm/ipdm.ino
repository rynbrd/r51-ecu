#include <AUnit.h>
#include <Arduino.h>
#include <Faker.h>
#include <Test.h>
#include <Vehicle.h>

namespace R51 {

using namespace aunit;
using ::Canny::Frame;
using ::Faker::FakeClock;
using ::Faker::FakeGPIO;

test(IPDMTest, IgnoreIncorrectID) {
    FakeYield yield;
    Frame f(0x624, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    IPDM ipdm;
    ipdm.handle(f);
    ipdm.emit(yield);
    assertSize(yield, 0);
}

test(IPDMTest, IgnoreIncorrectSize) {
    FakeYield yield;
    Frame f(0x625, 0, {0x00, 0x00, 0x00});

    IPDM ipdm;
    ipdm.handle(f);
    ipdm.emit(yield);
    assertSize(yield, 0);
}

test(IPDMTest, Tick) {
    FakeClock clock;
    FakeYield yield;

    IPDM ipdm(200, &clock);
    ipdm.emit(yield);
    assertSize(yield, 0);

    Event expect((uint8_t)SubSystem::IPDM, (uint8_t)IPDMEvent::POWER_STATE, {0x00});
    clock.set(200);
    ipdm.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
}

test(IPDMTest, Defog) {
    FakeYield yield;
    Frame f(0x625, 0, {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    IPDM ipdm;
    ipdm.handle(f);
    ipdm.emit(yield);
    
    Event expect((uint8_t)SubSystem::IPDM, (uint8_t)IPDMEvent::POWER_STATE, {0x40});
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
}

test(IPDMTest, HighBeams) {
    FakeYield yield;
    Frame f(0x625, 0, {0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    IPDM ipdm;
    ipdm.handle(f);
    ipdm.emit(yield);

    Event expect((uint8_t)SubSystem::IPDM, (uint8_t)IPDMEvent::POWER_STATE, {0x01});
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
}

test(IPDMTest, LowBeams) {
    FakeYield yield;
    Frame f(0x625, 0, {0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    IPDM ipdm;
    ipdm.handle(f);
    ipdm.emit(yield);

    Event expect((uint8_t)SubSystem::IPDM, (uint8_t)IPDMEvent::POWER_STATE, {0x02});
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
}

test(IPDMTest, FogLights) {
    FakeYield yield;
    Frame f(0x625, 0, {0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    IPDM ipdm;
    ipdm.handle(f);
    ipdm.emit(yield);

    Event expect((uint8_t)SubSystem::IPDM, (uint8_t)IPDMEvent::POWER_STATE, {0x08});
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
}

test(IPDMTest, RunningLights) {
    FakeYield yield;
    Frame f(0x625, 0, {0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    IPDM ipdm;
    ipdm.handle(f);
    ipdm.emit(yield);

    Event expect((uint8_t)SubSystem::IPDM, (uint8_t)IPDMEvent::POWER_STATE, {0x04});
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
}

test(IPDMTest, ACCompressor) {
    FakeYield yield;
    Frame f(0x625, 0, {0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    IPDM ipdm;
    ipdm.handle(f);
    ipdm.emit(yield);

    Event expect((uint8_t)SubSystem::IPDM, (uint8_t)IPDMEvent::POWER_STATE, {0x80});
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
}

test(DefogTest, Trigger) {
    FakeYield yield;
    FakeClock clock;
    FakeGPIO gpio;
    Defog defog(1, 200, &clock, &gpio);
    Event event((uint8_t)SubSystem::IPDM, (uint8_t)IPDMEvent::TOGGLE_DEFOG);

    // Ensure default state is off.
    defog.emit(yield);
    assertSize(yield, 0);
    assertEqual(gpio.digitalRead(1), 0);

    // Toggle the defog heater.
    clock.set(1);
    defog.handle(event);
    defog.emit(yield);
    assertSize(yield, 0);
    assertEqual(gpio.digitalRead(1), 1);

    // Ensure state is on after only part of the time elapses.
    clock.set(200);
    defog.emit(yield);
    assertSize(yield, 0);
    assertEqual(gpio.digitalRead(1), 1);

    // Ensure state is off after the entire time elapses.
    clock.set(201);
    defog.emit(yield);
    assertSize(yield, 0);
    assertEqual(gpio.digitalRead(1), 0);

    // State stays the same.
    clock.set(1000);
    defog.emit(yield);
    assertSize(yield, 0);
    assertEqual(gpio.digitalRead(1), 0);

    // Trigger it again.
    clock.set(1001);
    defog.handle(event);
    defog.emit(yield);
    assertSize(yield, 0);
    assertEqual(gpio.digitalRead(1), 1);

    clock.set(1200);
    defog.emit(yield);
    assertSize(yield, 0);
    assertEqual(gpio.digitalRead(1), 1);

    clock.set(1201);
    defog.emit(yield);
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
