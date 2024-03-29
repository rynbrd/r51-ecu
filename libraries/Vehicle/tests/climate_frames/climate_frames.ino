#include <AUnit.h>
#include <Arduino.h>
#include <Vehicle.h>

namespace R51 {

using namespace aunit;
using ::Canny::CAN20Frame;

bool checkFrame(const CAN20Frame& a, const CAN20Frame& b) {
    if (a == b) {
        return true;
    }
    a.printTo(SERIAL_PORT_MONITOR);
    SERIAL_PORT_MONITOR.print(" != ");
    b.printTo(SERIAL_PORT_MONITOR);
    SERIAL_PORT_MONITOR.println();
    return false;
}

test(ClimateSystemControlFrameTest, Init) {
    ClimateSystemControlFrame c;
    CAN20Frame expect(0x540, 0, (uint8_t[]){0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, Ready) {
    ClimateSystemControlFrame c(true);
    CAN20Frame expect(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});

    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, InitThenReady) {
    ClimateSystemControlFrame c;

    CAN20Frame expect(0x540, 0, (uint8_t[]){0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkFrame(c, expect));

    c.ready();
    expect.data((uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, TurnOff) {
    ClimateSystemControlFrame c(true);

    c.turnOff();
    CAN20Frame expect(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x84, 0x00});
    assertTrue(checkFrame(c, expect));

    c.turnOff();
    expect.data((uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, ToggleAuto) {
    ClimateSystemControlFrame c(true);

    c.toggleAuto();
    CAN20Frame expect(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00});
    assertTrue(checkFrame(c, expect));

    c.toggleAuto();
    expect.data((uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, ToggleAC) {
    ClimateSystemControlFrame c(true);

    c.toggleAC();
    CAN20Frame expect(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x08, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));

    c.toggleAC();
    expect.data((uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, ToggleDual) {
    ClimateSystemControlFrame c(true);

    c.toggleDual();
    CAN20Frame expect(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00});
    assertTrue(checkFrame(c, expect));

    c.toggleDual();
    expect.data((uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, CycleMode) {
    ClimateSystemControlFrame c(true);

    c.cycleMode();
    CAN20Frame expect(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00});
    assertTrue(checkFrame(c, expect));

    c.cycleMode();
    expect.data((uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, ToggleDefog) {
    ClimateSystemControlFrame c(true);

    c.toggleDefog();
    CAN20Frame expect(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00});
    assertTrue(checkFrame(c, expect));

    c.toggleDefog();
    expect.data((uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, IncDriverTemp) {
    ClimateSystemControlFrame c(true);

    c.incDriverTemp();
    CAN20Frame expect(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x01, 0x00, 0x20, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));

    c.incDriverTemp();
    expect.data((uint8_t[]){0x60, 0x40, 0x00, 0x02, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, DecDriverTemp) {
    ClimateSystemControlFrame c(true);

    c.decDriverTemp();
    CAN20Frame expect(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0xFF, 0x00, 0x20, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));

    c.decDriverTemp();
    expect.data((uint8_t[]){0x60, 0x40, 0x00, 0xFE, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, IncPassengerTemp) {
    ClimateSystemControlFrame c(true);

    c.incPassengerTemp();
    CAN20Frame expect(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x01, 0x20, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));

    c.incPassengerTemp();
    expect.data((uint8_t[]){0x60, 0x40, 0x00, 0x00, 0x02, 0x00, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, DecPassengerTemp) {
    ClimateSystemControlFrame c(true);

    c.decPassengerTemp();
    CAN20Frame expect(0x540, 0, (uint8_t[]){0x60, 0x40, 0x00, 0x00, 0xFF, 0x20, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));

    c.decPassengerTemp();
    expect.data((uint8_t[]){0x60, 0x40, 0x00, 0x00, 0xFE, 0x00, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateFanControlFrameTest, Init) {
    ClimateFanControlFrame c;
    CAN20Frame expect(0x541, 0, (uint8_t[]){0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    assertTrue(checkFrame(c, expect));
}

test(ClimateFanControlFrameTest, Ready) {
    ClimateFanControlFrame c(true);

    CAN20Frame expect(0x541, 0, (uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateFanControlFrameTest, InitThenReady) {
    ClimateFanControlFrame c;

    CAN20Frame expect(0x541, 0, (uint8_t[]){0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkFrame(c, expect));

    c.ready();
    expect.data((uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateFanControlFrameTest, ToggleRecirculate) {
    ClimateFanControlFrame c(true);

    c.toggleRecirculate();
    CAN20Frame expect(0x541, 0, (uint8_t[]){0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkFrame(c, expect));

    c.toggleRecirculate();
    expect.data((uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateFanControlFrameTest, IncFanSpeed) {
    ClimateFanControlFrame c(true);

    c.incFanSpeed();
    CAN20Frame expect(0x541, 0, (uint8_t[]){0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkFrame(c, expect));

    c.incFanSpeed();
    expect.data((uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateFanControlFrameTest, DecFanSpeed) {
    ClimateFanControlFrame c(true);

    c.decFanSpeed();
    CAN20Frame expect(0x541, 0, (uint8_t[]){0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkFrame(c, expect));

    c.decFanSpeed();
    expect.data((uint8_t[]){0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkFrame(c, expect));
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
