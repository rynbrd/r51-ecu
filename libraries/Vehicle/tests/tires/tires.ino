#include <AUnit.h>
#include <Arduino.h>
#include <Faker.h>
#include <Test.h>
#include <Vehicle.h>

namespace R51 {

using namespace aunit;
using ::Canny::Frame;
using ::Faker::FakeClock;

class FakeConfigStore : public ConfigStore {
    public:
        FakeConfigStore(bool valid) : valid(valid), map{0, 1, 2, 3} {}
        FakeConfigStore(bool valid, const uint8_t (&map)[4]) : valid(valid) {
            for (size_t i = 0; i < 4; i++) {
                this->map[i] = map[i];
            }
        }

        Error loadTireMap(uint8_t* dest) override {
            if (valid) {
                memcpy(dest, map, 4);
                return ConfigStore::SET;
            }
            return ConfigStore::UNSET;
        }

        Error saveTireMap(uint8_t* src) override {
            if (valid) {
                memcpy(map, src, 4);
                return ConfigStore::SET;
            }
            return ConfigStore::UNSET;
        }

        bool valid;
        uint8_t map[4];
};

test(TirePressureStateTest, IgnoreIncorrectID) {
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

    TirePressureState tire(nullptr, 200, &clock);
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

test(TirePressureStateTest, Request) {
    FakeYield yield;
    TirePressureState tire;
    Frame f;
    Event control = Event(
        (uint8_t)SubSystem::TIRE,
        (uint8_t)TireEvent::REQUEST);
    Event expect = Event(
        (uint8_t)SubSystem::TIRE,
        (uint8_t)TireEvent::PRESSURE_STATE,
        {0x01, 0x02, 0x03, 0x04});

    // Populate initial values from frame.
    f = Frame(0x385, 0, {0x84, 0x0C, 0x01, 0x02, 0x03, 0x04, 0x00, 0xF0});
    tire.handle(f);
    tire.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Request the current values.
    tire.handle(control);
    tire.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Ensure additional events are not sent.
    tire.emit(yield);
    assertSize(yield, 0);
}

test(TirePressureStateTest, LoadInitialInvalidMap) {
    FakeYield yield;
    FakeConfigStore config(false);
    TirePressureState tire(&config);
    Frame f = Frame(0x385, 0, {0x84, 0x0C, 0x01, 0x02, 0x03, 0x04, 0x00, 0xF0});
    Event expect = Event(
        (uint8_t)SubSystem::TIRE,
        (uint8_t)TireEvent::PRESSURE_STATE,
        {0x01, 0x02, 0x03, 0x04});

    // Ensure values are in the order provided by the frame.
    tire.handle(f);
    tire.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();
}

test(TirePressureStateTest, LoadInitialValidMap) {
    FakeYield yield;
    FakeConfigStore config(true, {0, 2, 1, 3});
    TirePressureState tire(&config);
    Frame f = Frame(0x385, 0, {0x84, 0x0C, 0x01, 0x02, 0x03, 0x04, 0x00, 0xF0});
    Event expect = Event(
        (uint8_t)SubSystem::TIRE,
        (uint8_t)TireEvent::PRESSURE_STATE,
        {0x01, 0x03, 0x02, 0x04});

    // Ensure values are in the order provided by the frame.
    tire.handle(f);
    tire.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();
}

test(TirePressureStateTest, SwapAndSaveMap) {
    FakeYield yield;
    FakeConfigStore config(true);
    TirePressureState tire(&config);
    Frame f = Frame(0x385, 0, {0x84, 0x0C, 0x01, 0x02, 0x03, 0x04, 0x00, 0xF0});
    Event expect;

    // Ensure values are in the order provided by the frame.
    tire.handle(f);
    tire.emit(yield);
    expect = Event(
        (uint8_t)SubSystem::TIRE,
        (uint8_t)TireEvent::PRESSURE_STATE,
        {0x01, 0x02, 0x03, 0x04});
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Swap tire positions.
    Event control = Event(
        (uint8_t)SubSystem::TIRE,
        (uint8_t)TireEvent::SWAP_POSITION,
        {0x03});
    tire.handle(control);
    tire.emit(yield);
    expect = Event(
        (uint8_t)SubSystem::TIRE,
        (uint8_t)TireEvent::PRESSURE_STATE,
        {0x04, 0x02, 0x03, 0x01});
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    uint8_t map[4] = {3, 1, 2, 0};
    assertEqual(memcmp(map, config.map, 4), 0);
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
