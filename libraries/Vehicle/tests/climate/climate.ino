#include <AUnit.h>
#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include <Core.h>
#include <Faker.h>
#include <Test.h>
#include <Vehicle.h>

namespace R51 {

using namespace aunit;
using ::Canny::CAN20Frame;
using ::Faker::FakeClock;

#define assertYieldFrame(control, frame) ({\
    climate.handle(MessageView(&control), yield); \
    climate.emit(yield); \
    assertSize(yield, 1); \
    assertIsCANFrame(yield.messages()[0], frame); \
    yield.clear(); \
})

#define assertNoYield(control) ({\
    climate.handle(MessageView(&control), yield); \
    climate.emit(yield); \
    assertSize(yield, 0); \
})

class ClimateTest : public TestOnce {
    public:
        void setup() {
            TestOnce::setup();
            clock.set(1);
            yield.clear();
        }

        void initClimate(Climate* climate) {
            climate->emit(yield);
            clock.delay(1000);
            climate->emit(yield);
            yield.clear();
        }

        void enableClimate(Climate* climate) {
            CAN20Frame state54A(0x54A, 0, (uint8_t[]){0x3C, 0x3E, 0x7F, 0x80, 0x3C, 0x41, 0x00, 0x58});
            CAN20Frame state54B(0x54B, 0, (uint8_t[]){0x59, 0x8C, 0x05, 0x24, 0x00, 0x00, 0x00, 0x02});
            climate->handle(MessageView(&state54A), yield);
            climate->handle(MessageView(&state54B), yield);
            climate->emit(yield);
            yield.clear();
        }

        void testFanSpeed(uint8_t can_fan_speed, uint8_t event_fan_speed) {
            Climate climate(0, &clock);
            initClimate(&climate);
            enableClimate(&climate);

            CAN20Frame state(0x54B, 0, (uint8_t[]){0x59, 0x8C, can_fan_speed, 0x24, 0x00, 0x00, 0x00, 0x02});
            ClimateAirflowState event;
            event.fan_speed(event_fan_speed);
            event.feet(1);

            climate.handle(MessageView(&state), yield);
            assertSize(yield, 1);
            assertIsEvent(yield.messages()[0], event);
        }

        FakeClock clock;
        FakeYield yield;
};

testF(ClimateTest, Init) {
    Climate climate(0, &clock);

    CAN20Frame init540(0x540, 0, (uint8_t[]){0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    CAN20Frame init541(0x541, 0, (uint8_t[]){0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    CAN20Frame ready540(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    CAN20Frame ready541(0x541, 0, (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    for (int i = 0; i < 4; i++) {
        clock.delay(100);
        climate.emit(yield);
        assertSize(yield, 2);
        assertIsCANFrame(yield.messages()[0], init540);
        assertIsCANFrame(yield.messages()[1], init541);
        yield.clear();
    }
    clock.delay(100);
    climate.emit(yield);
    assertSize(yield, 2);
    assertIsCANFrame(yield.messages()[0], ready540);
    assertIsCANFrame(yield.messages()[1], ready541);
}

testF(ClimateTest, RequestSystemState) {
    Climate climate(0, &clock);
    initClimate(&climate);
    enableClimate(&climate);

    RequestCommand control(SubSystem::CLIMATE, (uint8_t)ClimateEvent::SYSTEM_STATE);
    Event expect(
        (uint8_t)SubSystem::CLIMATE,
        (uint8_t)ClimateEvent::SYSTEM_STATE,
        (uint8_t[]){0x0D});

    climate.handle(MessageView(&control), yield);
    climate.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();
}

testF(ClimateTest, RequestAirflowState) {
    Climate climate(0, &clock);
    initClimate(&climate);
    enableClimate(&climate);

    RequestCommand control(SubSystem::CLIMATE, (uint8_t)ClimateEvent::AIRFLOW_STATE);
    Event expect(
        (uint8_t)SubSystem::CLIMATE,
        (uint8_t)ClimateEvent::AIRFLOW_STATE,
        (uint8_t[]){0x03, 0x02});

    climate.handle(MessageView(&control), yield);
    climate.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();
}

testF(ClimateTest, RequestTempState) {
    Climate climate(0, &clock);
    initClimate(&climate);
    enableClimate(&climate);

    RequestCommand control(SubSystem::CLIMATE, (uint8_t)ClimateEvent::TEMP_STATE);
    Event expect(
        (uint8_t)SubSystem::CLIMATE,
        (uint8_t)ClimateEvent::TEMP_STATE,
        (uint8_t[]){0x3C, 0x41, 0x58, 0x01});

    climate.handle(MessageView(&control), yield);
    climate.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
    yield.clear();
}

testF(ClimateTest, RequestSubSystem) {
    Climate climate(0, &clock);
    initClimate(&climate);
    enableClimate(&climate);

    RequestCommand control(SubSystem::CLIMATE);
    Event expect_temp(
        (uint8_t)SubSystem::CLIMATE,
        (uint8_t)ClimateEvent::TEMP_STATE,
        (uint8_t[]){0x3C, 0x41, 0x58, 0x01});
    Event expect_system(
        (uint8_t)SubSystem::CLIMATE,
        (uint8_t)ClimateEvent::SYSTEM_STATE,
        (uint8_t[]){0x0D});
    Event expect_airflow(
        (uint8_t)SubSystem::CLIMATE,
        (uint8_t)ClimateEvent::AIRFLOW_STATE,
        (uint8_t[]){0x03, 0x02});

    climate.handle(MessageView(&control), yield);
    climate.emit(yield);
    assertSize(yield, 3);
    assertIsEvent(yield.messages()[0], expect_temp);
    assertIsEvent(yield.messages()[1], expect_system);
    assertIsEvent(yield.messages()[2], expect_airflow);
    yield.clear();
}

testF(ClimateTest, RequestAll) {
    Climate climate(0, &clock);
    initClimate(&climate);
    enableClimate(&climate);

    RequestCommand control;
    Event expect_temp(
        (uint8_t)SubSystem::CLIMATE,
        (uint8_t)ClimateEvent::TEMP_STATE,
        (uint8_t[]){0x3C, 0x41, 0x58, 0x01});
    Event expect_system(
        (uint8_t)SubSystem::CLIMATE,
        (uint8_t)ClimateEvent::SYSTEM_STATE,
        (uint8_t[]){0x0D});
    Event expect_airflow(
        (uint8_t)SubSystem::CLIMATE,
        (uint8_t)ClimateEvent::AIRFLOW_STATE,
        (uint8_t[]){0x03, 0x02});

    climate.handle(MessageView(&control), yield);
    climate.emit(yield);
    assertSize(yield, 3);
    assertIsEvent(yield.messages()[0], expect_temp);
    assertIsEvent(yield.messages()[1], expect_system);
    assertIsEvent(yield.messages()[2], expect_airflow);
    yield.clear();
}

testF(ClimateTest, TurnOff) {
    Climate climate(0, &clock);
    initClimate(&climate);

    CAN20Frame expect;
    Event control((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::TURN_OFF_CMD);

    expect = CAN20Frame(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x84, 0x00});
    assertYieldFrame(control, expect);

    expect = CAN20Frame(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertYieldFrame(control, expect);
}

testF(ClimateTest, ToggleAuto) {
    Climate climate(0, &clock);
    initClimate(&climate);

    Event control((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::TOGGLE_AUTO_CMD);
    CAN20Frame expect;

    expect = CAN20Frame(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00});
    assertYieldFrame(control, expect);

    expect = CAN20Frame(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertYieldFrame(control, expect);
}

testF(ClimateTest, ToggleAc) {
    Climate climate(0, &clock);
    initClimate(&climate);

    Event control((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::TOGGLE_AC_CMD);
    CAN20Frame expect;

    expect = CAN20Frame(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x08, 0x04, 0x00});
    assertYieldFrame(control, expect);

    expect = CAN20Frame(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertYieldFrame(control, expect);
}

testF(ClimateTest, ToggleDual) {
    Climate climate(0, &clock);
    initClimate(&climate);

    Event control((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::TOGGLE_DUAL_CMD);
    CAN20Frame expect;

    expect = CAN20Frame(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00});
    assertYieldFrame(control, expect);

    expect = CAN20Frame(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertYieldFrame(control, expect);
}

testF(ClimateTest, CycleMode) {
    Climate climate(0, &clock);
    initClimate(&climate);

    Event control((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::CYCLE_AIRFLOW_MODE_CMD);
    CAN20Frame expect;

    expect = CAN20Frame(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00});
    assertYieldFrame(control, expect);

    expect = CAN20Frame(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertYieldFrame(control, expect);
}

testF(ClimateTest, ToggleDefog) {
    Climate climate(0, &clock);
    initClimate(&climate);

    Event control((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::TOGGLE_DEFOG_CMD);
    CAN20Frame expect;

    expect = CAN20Frame(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00});
    assertYieldFrame(control, expect);

    expect = CAN20Frame(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertYieldFrame(control, expect);
}

testF(ClimateTest, ToggleRecirculate) {
    Climate climate(0, &clock);
    initClimate(&climate);

    Event control((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::TOGGLE_RECIRCULATE_CMD);
    CAN20Frame expect;

    expect = CAN20Frame(0x541, 0, (uint8_t[]){0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertYieldFrame(control, expect);

    expect = CAN20Frame(0x541, 0, (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertYieldFrame(control, expect);
}

testF(ClimateTest, TriggerFanSpeedUp) {
    Climate climate(0, &clock);
    initClimate(&climate);

    Event control((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::INC_FAN_SPEED_CMD);
    CAN20Frame expect;

    expect = CAN20Frame(0x541, 0, (uint8_t[]){0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertYieldFrame(control, expect);

    expect = CAN20Frame(0x541, 0, (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertYieldFrame(control, expect);
}

testF(ClimateTest, TriggerFanSpeedDown) {
    Climate climate(0, &clock);
    initClimate(&climate);

    Event control((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::DEC_FAN_SPEED_CMD);
    CAN20Frame expect;

    expect = CAN20Frame(0x541, 0, (uint8_t[]){0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertYieldFrame(control, expect);

    expect = CAN20Frame(0x541, 0, (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertYieldFrame(control, expect);
}

testF(ClimateTest, FanSpeedZero) {
    testFanSpeed(0x00, 0x00);
}

testF(ClimateTest, FanSpeedOne) {
    testFanSpeed(0x01, 0x01);
}

testF(ClimateTest, FanSpeedMiddle) {
    testFanSpeed(0x08, 0x04);
}

testF(ClimateTest, FanSpeedMiddleMinusOne) {
    testFanSpeed(0x07, 0x04);
}

testF(ClimateTest, FanSpeedMaxMinusOne) { 
    testFanSpeed(0x0E, 0x07);
}

testF(ClimateTest, FanSpeedMax) {
    testFanSpeed(0x0F, 0x07);
}

testF(ClimateTest, FanSpeedMiddleExtraBits) {
    testFanSpeed(0x28, 0x04);
}

testF(ClimateTest, FanSpeedMaxExtraBits) {
    testFanSpeed(0x2F, 0x07);
}

testF(ClimateTest, TriggerDriverTempWhenOn) {
    Climate climate(0, &clock);
    initClimate(&climate);
    enableClimate(&climate);

    Event control;
    CAN20Frame expect;

    // decrease temp
    control = Event((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::DEC_DRIVER_TEMP_CMD);
    expect = CAN20Frame(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0xFF, 0x00, 0x20, 0x04, 0x00});
    assertYieldFrame(control, expect);

    // increase temp
    control = Event((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::INC_DRIVER_TEMP_CMD);
    expect = CAN20Frame(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertYieldFrame(control, expect);

    // increase temp
    control = Event((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::INC_DRIVER_TEMP_CMD);
    expect = CAN20Frame(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x01, 0x00, 0x20, 0x04, 0x00});
    assertYieldFrame(control, expect);

    // decrease temp
    control = Event((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::DEC_DRIVER_TEMP_CMD);
    expect = CAN20Frame(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertYieldFrame(control, expect);
}

testF(ClimateTest, TriggerDriverTempWhenOff) {
    Climate climate(0, &clock);
    initClimate(&climate);

    Event control;

    // increase temp
    control = Event((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::INC_DRIVER_TEMP_CMD);
    assertNoYield(control);

    // decrease temp
    control = Event((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::DEC_DRIVER_TEMP_CMD);
    assertNoYield(control);
}

testF(ClimateTest, TriggerPassengerTempWhenOn) {
    Climate climate(0, &clock);
    initClimate(&climate);
    enableClimate(&climate);

    Event control;
    CAN20Frame expect;

    // decrease temp
    control = Event((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::DEC_PASSENGER_TEMP_CMD);
    expect = CAN20Frame(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0xFF, 0x20, 0x04, 0x00});
    assertYieldFrame(control, expect);

    // increase temp
    control = Event((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::INC_PASSENGER_TEMP_CMD);
    expect = CAN20Frame(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertYieldFrame(control, expect);

    // increase temp
    control = Event((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::INC_PASSENGER_TEMP_CMD);
    expect = CAN20Frame(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x01, 0x20, 0x04, 0x00});
    assertYieldFrame(control, expect);

    // decrease temp
    control = Event((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::DEC_PASSENGER_TEMP_CMD);
    expect = CAN20Frame(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertYieldFrame(control, expect);
}

testF(ClimateTest, TriggerPassengerTempWhenOff) {
    Climate climate(0, &clock);
    initClimate(&climate);

    Event control;

    // increase temp
    control = Event((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::INC_PASSENGER_TEMP_CMD);
    assertNoYield(control);

    // decrease temp
    control = Event((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::DEC_PASSENGER_TEMP_CMD);
    assertNoYield(control);
}

testF(ClimateTest, TickOffState) {
    Climate climate(100, &clock);
    initClimate(&climate);
    enableClimate(&climate);

    CAN20Frame state54A(0x54A, 0, (uint8_t[]){0x3C, 0x3E, 0x7F, 0x80, 0x00, 0x00, 0x00, 0x2C});
    CAN20Frame state54B(0x54B, 0, (uint8_t[]){0xF2, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x02});

    ClimateTempState temp;
    temp.outside_temp(0x2C);
    temp.units(UNITS_US);
    ClimateAirflowState airflow;
    ClimateSystemState system;
    system.mode(CLIMATE_SYSTEM_OFF);

    climate.handle(MessageView(&state54A), yield);
    climate.handle(MessageView(&state54B), yield);
    climate.emit(yield);
    assertSize(yield, 3);
    assertIsEvent(yield.messages()[0], temp);
    assertIsEvent(yield.messages()[1], system);
    assertIsEvent(yield.messages()[2], airflow);
    yield.clear();

    clock.delay(100);
    climate.emit(yield);
    assertSize(yield, 3);
    assertIsEvent(yield.messages()[0], temp);
    assertIsEvent(yield.messages()[1], system);
    assertIsEvent(yield.messages()[2], airflow);
    yield.clear();
}

}  // namespace 

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
