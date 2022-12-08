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

test(IllumTest, RequestAll) {
    FakeYield yield;
    Illum illum;

    RequestCommand request;
    IllumState expect;
    expect.illum(false);

    illum.handle(MessageView(&request), yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
}

test(IllumTest, RequestBCM) {
    FakeYield yield;
    Illum illum;

    RequestCommand request(SubSystem::BCM);
    IllumState expect;
    expect.illum(false);

    illum.handle(MessageView(&request), yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
}

test(IllumTest, RequestIllum) {
    FakeYield yield;
    Illum illum;

    Event event(SubSystem::IPDM, (uint8_t)IPDMEvent::POWER_STATE, {0xFF});
    illum.handle(MessageView(&event), yield);
    yield.clear();

    RequestCommand request(SubSystem::BCM, (uint8_t)BCMEvent::ILLUM_STATE);
    IllumState expect;
    expect.illum(true);

    illum.handle(MessageView(&request), yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
}

test(IllumTest, OnWhenLowBeam) {
    FakeYield yield;
    Illum illum;

    Event event(SubSystem::IPDM, (uint8_t)IPDMEvent::POWER_STATE, {0x02});
    IllumState expect;
    expect.illum(true);

    illum.handle(MessageView(&event), yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
}

test(IllumTest, OnWhenHighBeam) {
    FakeYield yield;
    Illum illum;

    Event event(SubSystem::IPDM, (uint8_t)IPDMEvent::POWER_STATE, {0x01});
    IllumState expect;
    expect.illum(true);

    illum.handle(MessageView(&event), yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
}

test(IllumTest, RemainOnWhenHighBeamToggle) {
    FakeYield yield;
    Illum illum;

    Event event(SubSystem::IPDM, (uint8_t)IPDMEvent::POWER_STATE, {0x02});
    IllumState expect;
    expect.illum(true);

    illum.handle(MessageView(&event), yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    event = Event(SubSystem::IPDM, (uint8_t)IPDMEvent::POWER_STATE, {0x01});
    illum.handle(MessageView(&event), yield);
    assertSize(yield, 0);
}

test(IllumTest, OnWhenBothBeamsOn) {
    FakeYield yield;
    Illum illum;

    Event event(SubSystem::IPDM, (uint8_t)IPDMEvent::POWER_STATE, {0x03});
    IllumState expect;
    expect.illum(true);

    illum.handle(MessageView(&event), yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
}

test(IllumTest, OffWhenBeamsOff) {
    FakeYield yield;
    Illum illum;

    Event event(SubSystem::IPDM, (uint8_t)IPDMEvent::POWER_STATE, {0x02});
    IllumState expect;
    expect.illum(true);

    illum.handle(MessageView(&event), yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    event = Event(SubSystem::IPDM, (uint8_t)IPDMEvent::POWER_STATE, {0x00});
    expect.illum(false);

    illum.handle(MessageView(&event), yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
}

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
    defrost.handle(MessageView(&event), yield);
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
    defrost.handle(MessageView(&event), yield);
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

test(TirePressureTest, IgnoreIncorrectID) {
    FakeYield yield;
    Frame f(0x384, 0, {0x84, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    TirePressure tire;
    tire.handle(MessageView(&f), yield);
    tire.emit(yield);
    assertSize(yield, 0);
}

test(TirePressureTest, IgnoreIncorrectSize) {
    FakeYield yield;
    Frame f(0x385, 0, {});

    TirePressure tire;
    tire.handle(MessageView(&f), yield);
    tire.emit(yield);
    assertSize(yield, 0);
}

test(TirePressureTest, Tick) {
    FakeClock clock;
    FakeYield yield;

    TirePressure tire(nullptr, 200, &clock);
    tire.emit(yield);
    assertSize(yield, 0);

    Event expect((uint8_t)SubSystem::BCM, (uint8_t)BCMEvent::TIRE_PRESSURE_STATE, {0x00, 0x00, 0x00, 0x00});
    clock.set(200);
    tire.emit(yield);
    assertIsEvent(yield.messages()[0], expect);
}

test(TirePressureTest, AllSet) {
    FakeYield yield;
    Frame f(0x385, 0, {0x84, 0x0C, 0x82, 0x84, 0x79, 0x77, 0x00, 0xF0});

    TirePressure tire;
    tire.handle(MessageView(&f), yield);
    tire.emit(yield);

    Event expect((uint8_t)SubSystem::BCM, (uint8_t)BCMEvent::TIRE_PRESSURE_STATE, {0x82, 0x84, 0x79, 0x77});
    assertIsEvent(yield.messages()[0], expect);
}

test(TirePressureTest, Tire1Set) {
    FakeYield yield;
    Frame f(0x385, 0, {0x84, 0x0C, 0x82, 0x00, 0x00, 0x00, 0x00, 0x80});

    TirePressure tire;
    tire.handle(MessageView(&f), yield);
    tire.emit(yield);

    Event expect((uint8_t)SubSystem::BCM, (uint8_t)BCMEvent::TIRE_PRESSURE_STATE, {0x82, 0x00, 0x00, 0x00});
    assertIsEvent(yield.messages()[0], expect);
}

test(TirePressureTest, Tire2Set) {
    FakeYield yield;
    Frame f(0x385, 0, {0x84, 0x0C, 0x00, 0xA0, 0x00, 0x00, 0x00, 0x40});

    TirePressure tire;
    tire.handle(MessageView(&f), yield);
    tire.emit(yield);

    Event expect((uint8_t)SubSystem::BCM, (uint8_t)BCMEvent::TIRE_PRESSURE_STATE, {0x00, 0xA0, 0x00, 0x00});
    assertIsEvent(yield.messages()[0], expect);
}

test(TirePressureTest, Tire3Set) {
    FakeYield yield;
    Frame f(0x385, 0, {0x84, 0x0C, 0x00, 0x00, 0x75, 0x00, 0x00, 0x20});

    TirePressure tire;
    tire.handle(MessageView(&f), yield);
    tire.emit(yield);

    Event expect((uint8_t)SubSystem::BCM, (uint8_t)BCMEvent::TIRE_PRESSURE_STATE, {0x00, 0x00, 0x75, 0x00});
    assertIsEvent(yield.messages()[0], expect);
}

test(TirePressureTest, Tire4Set) {
    FakeYield yield;
    Frame f(0x385, 0, {0x84, 0x0C, 0x00, 0x00, 0x00, 0x77, 0x00, 0x10});

    TirePressure tire;
    tire.handle(MessageView(&f), yield);
    tire.emit(yield);

    Event expect((uint8_t)SubSystem::BCM, (uint8_t)BCMEvent::TIRE_PRESSURE_STATE, {0x00, 0x00, 0x00, 0x77});
    assertIsEvent(yield.messages()[0], expect);
}

test(TirePressureTest, Swap) {
    FakeYield yield;
    TirePressure tire;
    Frame f;
    Event control;
    Event expect;

    // Populate initial values from frame.
    f = Frame(0x385, 0, {0x84, 0x0C, 0x01, 0x02, 0x03, 0x04, 0x00, 0xF0});
    tire.handle(MessageView(&f), yield);
    tire.emit(yield);
    expect = Event((uint8_t)SubSystem::BCM, (uint8_t)BCMEvent::TIRE_PRESSURE_STATE, {0x01, 0x02, 0x03, 0x04});
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Send control frame to swap positions.
    control = Event((uint8_t)SubSystem::BCM, (uint8_t)BCMEvent::TIRE_SWAP_CMD, {0x03});
    tire.handle(MessageView(&control), yield);
    tire.emit(yield);
    expect = Event((uint8_t)SubSystem::BCM, (uint8_t)BCMEvent::TIRE_PRESSURE_STATE, {0x04, 0x02, 0x03, 0x01});
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Send new values from frame.
    f = Frame(0x385, 0, {0x84, 0x0C, 0x11, 0x12, 0x13, 0x14, 0x00, 0xF0});
    tire.handle(MessageView(&f), yield);
    tire.emit(yield);
    expect = Event((uint8_t)SubSystem::BCM, (uint8_t)BCMEvent::TIRE_PRESSURE_STATE, {0x14, 0x12, 0x13, 0x11});
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Send control frame to swap positions again.
    control = Event((uint8_t)SubSystem::BCM, (uint8_t)BCMEvent::TIRE_SWAP_CMD, {0x23});
    tire.handle(MessageView(&control), yield);
    tire.emit(yield);
    expect = Event((uint8_t)SubSystem::BCM, (uint8_t)BCMEvent::TIRE_PRESSURE_STATE, {0x14, 0x12, 0x11, 0x13});
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Send new values from frame.
    f = Frame(0x385, 0, {0x84, 0x0C, 0x21, 0x22, 0x23, 0x24, 0x00, 0xF0});
    tire.handle(MessageView(&f), yield);
    tire.emit(yield);
    expect = Event((uint8_t)SubSystem::BCM, (uint8_t)BCMEvent::TIRE_PRESSURE_STATE, {0x24, 0x22, 0x21, 0x23});
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();
}

test(TirePressureTest, RequestPressureTest) {
    FakeYield yield;
    TirePressure tire;
    Frame f;
    RequestCommand control(
        SubSystem::BCM,
        (uint8_t)BCMEvent::TIRE_PRESSURE_STATE);
    Event expect(
        (uint8_t)SubSystem::BCM,
        (uint8_t)BCMEvent::TIRE_PRESSURE_STATE,
        {0x01, 0x02, 0x03, 0x04});

    // Populate initial values from frame.
    f = Frame(0x385, 0, {0x84, 0x0C, 0x01, 0x02, 0x03, 0x04, 0x00, 0xF0});
    tire.handle(MessageView(&f), yield);
    tire.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Request the current values.
    tire.handle(MessageView(&control), yield);
    tire.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Ensure additional events are not sent.
    tire.emit(yield);
    assertSize(yield, 0);
}

test(TirePressureTest, RequestSubSystem) {
    FakeYield yield;
    TirePressure tire;
    Frame f;
    RequestCommand control(SubSystem::BCM);
    Event expect(
        (uint8_t)SubSystem::BCM,
        (uint8_t)BCMEvent::TIRE_PRESSURE_STATE,
        {0x01, 0x02, 0x03, 0x04});

    // Populate initial values from frame.
    f = Frame(0x385, 0, {0x84, 0x0C, 0x01, 0x02, 0x03, 0x04, 0x00, 0xF0});
    tire.handle(MessageView(&f), yield);
    tire.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Request the current values.
    tire.handle(MessageView(&control), yield);
    tire.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Ensure additional events are not sent.
    tire.emit(yield);
    assertSize(yield, 0);
}

test(TirePressureTest, RequestAll) {
    FakeYield yield;
    TirePressure tire;
    Frame f;
    RequestCommand control;
    Event expect(
        (uint8_t)SubSystem::BCM,
        (uint8_t)BCMEvent::TIRE_PRESSURE_STATE,
        {0x01, 0x02, 0x03, 0x04});

    // Populate initial values from frame.
    f = Frame(0x385, 0, {0x84, 0x0C, 0x01, 0x02, 0x03, 0x04, 0x00, 0xF0});
    tire.handle(MessageView(&f), yield);
    tire.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Request the current values.
    tire.handle(MessageView(&control), yield);
    tire.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Ensure additional events are not sent.
    tire.emit(yield);
    assertSize(yield, 0);
}

test(TirePressureTest, LoadInitialInvalidMap) {
    FakeYield yield;
    FakeConfigStore config(false);
    TirePressure tire(&config);
    Frame f = Frame(0x385, 0, {0x84, 0x0C, 0x01, 0x02, 0x03, 0x04, 0x00, 0xF0});
    Event expect = Event(
        (uint8_t)SubSystem::BCM,
        (uint8_t)BCMEvent::TIRE_PRESSURE_STATE,
        {0x01, 0x02, 0x03, 0x04});

    // Ensure values are in the order provided by the frame.
    tire.handle(MessageView(&f), yield);
    tire.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();
}

test(TirePressureTest, LoadInitialValidMap) {
    FakeYield yield;
    FakeConfigStore config(true, {0, 2, 1, 3});
    TirePressure tire(&config);
    Frame f = Frame(0x385, 0, {0x84, 0x0C, 0x01, 0x02, 0x03, 0x04, 0x00, 0xF0});
    Event expect = Event(
        (uint8_t)SubSystem::BCM,
        (uint8_t)BCMEvent::TIRE_PRESSURE_STATE,
        {0x01, 0x03, 0x02, 0x04});

    // Ensure values are in the order provided by the frame.
    tire.handle(MessageView(&f), yield);
    tire.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();
}

test(TirePressureTest, SwapAndSaveMap) {
    FakeYield yield;
    FakeConfigStore config(true);
    TirePressure tire(&config);
    Frame f = Frame(0x385, 0, {0x84, 0x0C, 0x01, 0x02, 0x03, 0x04, 0x00, 0xF0});
    Event expect;

    // Ensure values are in the order provided by the frame.
    tire.handle(MessageView(&f), yield);
    tire.emit(yield);
    expect = Event(
        (uint8_t)SubSystem::BCM,
        (uint8_t)BCMEvent::TIRE_PRESSURE_STATE,
        {0x01, 0x02, 0x03, 0x04});
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();

    // Swap tire positions.
    Event control = Event(
        (uint8_t)SubSystem::BCM,
        (uint8_t)BCMEvent::TIRE_SWAP_CMD,
        {0x03});
    tire.handle(MessageView(&control), yield);
    tire.emit(yield);
    expect = Event(
        (uint8_t)SubSystem::BCM,
        (uint8_t)BCMEvent::TIRE_PRESSURE_STATE,
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
