#include <AUnit.h>
#include <Arduino.h>
#include <Test.h>
#include <Vehicle.h>

namespace R51 {

using namespace aunit;

test(ClimateTempStateTest, Getters) {
    ClimateTempState event;
    event.data[0] = 0x40;
    event.data[1] = 0x41;
    event.data[2] = 0x49;
    event.data[3] = 0x01;

    assertEqual(event.driver_temp(), 0x40);
    assertEqual(event.passenger_temp(), 0x41);
    assertEqual(event.outside_temp(), 0x49);
    assertEqual(event.units(), UNITS_US);
}

test(ClimateTempStateTest, Setters) {
    ClimateTempState event;
    event.driver_temp(0x40);
    event.passenger_temp(0x41);
    event.outside_temp(0x49);
    event.units(UNITS_US);

    Event expect((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::TEMP_STATE, {0x40, 0x41, 0x49, 0x01});
    assertPrintablesEqual(event, expect);
}

test(ClimateAirflowStateTest, Getters) {
    ClimateAirflowState event;
    event.data[0] = 0x02;
    event.data[1] = 0x06;

    assertEqual(event.fan_speed(), 2);
    assertEqual(event.face(), false);
    assertEqual(event.feet(), true);
    assertEqual(event.windshield(), true);
    assertEqual(event.recirculate(), false);
}

test(ClimateAirflowStateTest, Setters) {
    ClimateAirflowState event;
    event.fan_speed(7);
    event.face(true);
    event.recirculate(true);

    Event expect((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::AIRFLOW_STATE, {0x07, 0x09});
    assertPrintablesEqual(event, expect);
}

test(ClimateSystemStateTest, GetOff) {
    ClimateSystemState event;
    event.data[0] = 0x00;

    assertEqual(event.mode(), CLIMATE_SYSTEM_OFF);
    assertEqual(event.ac(), false);
    assertEqual(event.dual(), false);
}

test(ClimateSystemStateTest, GetAutoAC) {
    ClimateSystemState event;
    event.data[0] = 0x05;

    assertEqual(event.mode(), CLIMATE_SYSTEM_AUTO);
    assertEqual(event.ac(), true);
    assertEqual(event.dual(), false);
}

test(ClimateSystemStateTest, GetManualDual) {
    ClimateSystemState event;
    event.data[0] = 0x0A;

    assertEqual(event.mode(), CLIMATE_SYSTEM_MANUAL);
    assertEqual(event.ac(), false);
    assertEqual(event.dual(), true);
}

test(ClimateSystemStateTest, SetOff) {
    ClimateSystemState event;
    event.data[0] = 0x05;
    event.mode(CLIMATE_SYSTEM_OFF);
    event.ac(false);
    event.dual(false);

    Event expect((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::SYSTEM_STATE, {0x00});
    assertPrintablesEqual(event, expect);
}

test(ClimateSystemStateTest, SetAutoAC) {
    ClimateSystemState event;
    event.mode(CLIMATE_SYSTEM_AUTO);
    event.ac(true);

    Event expect((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::SYSTEM_STATE, {0x05});
    assertPrintablesEqual(event, expect);
}

test(ClimateSystemStateTest, SetDefogACDual) {
    ClimateSystemState event;
    event.mode(CLIMATE_SYSTEM_DEFOG);
    event.ac(true);
    event.dual(true);

    Event expect((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::SYSTEM_STATE, {0x0F});
    assertPrintablesEqual(event, expect);
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
