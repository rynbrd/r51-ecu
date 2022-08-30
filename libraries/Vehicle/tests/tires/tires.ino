#include <AUnit.h>
#include <Arduino.h>
#include <Faker.h>
#include <Test.h>
#include <Vehicle.h>

namespace R51 {

using namespace aunit;
using ::Canny::Frame;
using ::Faker::FakeClock;

test(TirePressureState, IgnoreIncorrectID) {
    FakeYield yield;
    Frame f(0x384, 0, {0x84, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    TirePressureState tire;
    tire.handle(f);
    tire.emit(yield);
    assertSize(yield, 0);
}

test(TirePressureStateTest, IgnoreIncorrectSize) {
    FakeYield yield;
    Frame f(0x385, 0, {});

    TirePressureState tire;
    tire.handle(f);
    tire.emit(yield);
    assertSize(yield, 0);
}

test(TirePressureStateTest, Tick) {
    FakeClock clock;
    FakeYield yield;

    TirePressureState tire(200, &clock);
    tire.emit(yield);
    assertSize(yield, 0);

    Event expect((uint8_t)SubSystem::TIRE, (uint8_t)TireEvent::PRESSURE_STATE, {0x00, 0x00, 0x00, 0x00});
    clock.set(200);
    tire.emit(yield);
    assertIsEvent(yield.messages()[0], expect);
}

test(TirePressureStateTest, AllSet) {
    FakeYield yield;
    Frame f(0x385, 0, {0x84, 0x0C, 0x82, 0x84, 0x79, 0x77, 0x00, 0xF0});

    TirePressureState tire;
    tire.handle(f);
    tire.emit(yield);

    Event expect((uint8_t)SubSystem::TIRE, (uint8_t)TireEvent::PRESSURE_STATE, {0x82, 0x84, 0x79, 0x77});
    assertIsEvent(yield.messages()[0], expect);
}

test(TirePressureStateTest, Tire1Set) {
    FakeYield yield;
    Frame f(0x385, 0, {0x84, 0x0C, 0x82, 0x00, 0x00, 0x00, 0x00, 0x80});

    TirePressureState tire;
    tire.handle(f);
    tire.emit(yield);

    Event expect((uint8_t)SubSystem::TIRE, (uint8_t)TireEvent::PRESSURE_STATE, {0x82, 0x00, 0x00, 0x00});
    assertIsEvent(yield.messages()[0], expect);
}

test(TirePressureStateTest, Tire2Set) {
    FakeYield yield;
    Frame f(0x385, 0, {0x84, 0x0C, 0x00, 0xA0, 0x00, 0x00, 0x00, 0x40});

    TirePressureState tire;
    tire.handle(f);
    tire.emit(yield);

    Event expect((uint8_t)SubSystem::TIRE, (uint8_t)TireEvent::PRESSURE_STATE, {0x00, 0xA0, 0x00, 0x00});
    assertIsEvent(yield.messages()[0], expect);
}

test(TirePressureStateTest, Tire3Set) {
    FakeYield yield;
    Frame f(0x385, 0, {0x84, 0x0C, 0x00, 0x00, 0x75, 0x00, 0x00, 0x20});

    TirePressureState tire;
    tire.handle(f);
    tire.emit(yield);

    Event expect((uint8_t)SubSystem::TIRE, (uint8_t)TireEvent::PRESSURE_STATE, {0x00, 0x00, 0x75, 0x00});
    assertIsEvent(yield.messages()[0], expect);
}

test(TirePressureStateTest, Tire4Set) {
    FakeYield yield;
    Frame f(0x385, 0, {0x84, 0x0C, 0x00, 0x00, 0x00, 0x77, 0x00, 0x10});

    TirePressureState tire;
    tire.handle(f);
    tire.emit(yield);

    Event expect((uint8_t)SubSystem::TIRE, (uint8_t)TireEvent::PRESSURE_STATE, {0x00, 0x00, 0x00, 0x77});
    assertIsEvent(yield.messages()[0], expect);
}

test(TirePressureStateTest, Swap) {
    FakeYield yield;
    TirePressureState tire;
    Frame f;
    Event control;
    Event expect;

    // Populate initial values from frame.
    f = Frame(0x385, 0, {0x84, 0x0C, 0x01, 0x02, 0x03, 0x04, 0x00, 0xF0});
    tire.handle(f);
    tire.emit(yield);
    expect = Event((uint8_t)SubSystem::TIRE, (uint8_t)TireEvent::PRESSURE_STATE, {0x01, 0x02, 0x03, 0x04});
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Send control frame to swap positions.
    control = Event((uint8_t)SubSystem::TIRE, (uint8_t)TireEvent::SWAP_POSITION, {0x03});
    tire.handle(control);
    tire.emit(yield);
    expect = Event((uint8_t)SubSystem::TIRE, (uint8_t)TireEvent::PRESSURE_STATE, {0x04, 0x02, 0x03, 0x01});
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Send new values from frame.
    f = Frame(0x385, 0, {0x84, 0x0C, 0x11, 0x12, 0x13, 0x14, 0x00, 0xF0});
    tire.handle(f);
    tire.emit(yield);
    expect = Event((uint8_t)SubSystem::TIRE, (uint8_t)TireEvent::PRESSURE_STATE, {0x14, 0x12, 0x13, 0x11});
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Send control frame to swap positions again.
    control = Event((uint8_t)SubSystem::TIRE, (uint8_t)TireEvent::SWAP_POSITION, {0x23});
    tire.handle(control);
    tire.emit(yield);
    expect = Event((uint8_t)SubSystem::TIRE, (uint8_t)TireEvent::PRESSURE_STATE, {0x14, 0x12, 0x11, 0x13});
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Send new values from frame.
    f = Frame(0x385, 0, {0x84, 0x0C, 0x21, 0x22, 0x23, 0x24, 0x00, 0xF0});
    tire.handle(f);
    tire.emit(yield);
    expect = Event((uint8_t)SubSystem::TIRE, (uint8_t)TireEvent::PRESSURE_STATE, {0x24, 0x22, 0x21, 0x23});
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();
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
