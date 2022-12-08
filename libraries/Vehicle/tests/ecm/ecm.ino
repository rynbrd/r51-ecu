#include <AUnit.h>
#include <Arduino.h>
#include <Faker.h>
#include <Test.h>
#include <Vehicle.h>

namespace R51 {

using namespace aunit;
using ::Canny::Frame;
using ::Faker::FakeClock;

test(EngineTempStateTest, IgnoreIncorrectID) {
    FakeYield yield;
    Frame f(0x550, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    EngineTempState ecm;
    ecm.handle(MessageView(&f), yield);
    assertSize(yield, 0);
    ecm.emit(yield);
    assertSize(yield, 0);
}

test(EngineTempStateTest, IgnoreIncorrectSize) {
    FakeYield yield;
    Frame f(0x551, 0, {});

    EngineTempState ecm;
    ecm.handle(MessageView(&f), yield);
    ecm.emit(yield);
    assertSize(yield, 0);
}

test(EngineTempStateTest, Tick) {
    FakeClock clock;
    FakeYield yield;

    EngineTempState ecm(200, &clock);
    ecm.emit(yield);
    assertSize(yield, 0);

    Event expect((uint8_t)SubSystem::ECM, (uint8_t)ECMEvent::ENGINE_TEMP_STATE, {0x00});
    clock.set(200);
    ecm.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
}

test(EngineTempStateTest, PositiveTemp) {
    FakeYield yield;
    Frame f(0x551, 0, {0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    EngineTempState ecm;
    ecm.handle(MessageView(&f), yield);
    ecm.emit(yield);

    Event expect((uint8_t)SubSystem::ECM, (uint8_t)ECMEvent::ENGINE_TEMP_STATE, {0x29});
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
}

test(EngineTempStateTest, MaxTemp) {
    FakeYield yield;
    Frame f(0x551, 0, {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    EngineTempState ecm;
    ecm.handle(MessageView(&f), yield);
    ecm.emit(yield);

    Event expect((uint8_t)SubSystem::ECM, (uint8_t)ECMEvent::ENGINE_TEMP_STATE, {0xFF});
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
}

test(EngineTempStateTest, RequestPowerState) {
    FakeYield yield;
    Frame f(0x551, 0, {0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    RequestCommand control(
        SubSystem::ECM,
        (uint8_t)ECMEvent::ENGINE_TEMP_STATE);
    Event expect(
        (uint8_t)SubSystem::ECM,
        (uint8_t)ECMEvent::ENGINE_TEMP_STATE,
        {0x29});

    // Set the temperature from frame.
    EngineTempState ecm;
    ecm.handle(MessageView(&f), yield);
    ecm.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Request current state.
    ecm.handle(MessageView(&control), yield);
    ecm.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Ensure no additional events are sent.
    ecm.emit(yield);
    assertSize(yield, 0);
}

test(EngineTempStateTest, RequestAllSubSystem) {
    FakeYield yield;
    Frame f(0x551, 0, {0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    RequestCommand control(SubSystem::ECM);
    Event expect(
        (uint8_t)SubSystem::ECM,
        (uint8_t)ECMEvent::ENGINE_TEMP_STATE,
        {0x29});

    // Set the temperature from frame.
    EngineTempState ecm;
    ecm.handle(MessageView(&f), yield);
    ecm.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Request current state.
    ecm.handle(MessageView(&control), yield);
    ecm.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Ensure no additional events are sent.
    ecm.emit(yield);
    assertSize(yield, 0);
}

test(EngineTempStateTest, RequestAll) {
    FakeYield yield;
    Frame f(0x551, 0, {0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    RequestCommand control;
    Event expect(
        (uint8_t)SubSystem::ECM,
        (uint8_t)ECMEvent::ENGINE_TEMP_STATE,
        {0x29});

    // Set the temperature from frame.
    EngineTempState ecm;
    ecm.handle(MessageView(&f), yield);
    ecm.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Request current state.
    ecm.handle(MessageView(&control), yield);
    ecm.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Ensure no additional events are sent.
    ecm.emit(yield);
    assertSize(yield, 0);
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
