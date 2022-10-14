#include <AUnit.h>
#include <Arduino.h>
#include <Vehicle.h>

namespace R51 {

using namespace aunit;
using ::Canny::Frame;

bool checkFrame(const Frame& a, const Frame& b) {
    if (a == b) {
        return true;
    }
    SERIAL_PORT_MONITOR.print(a);
    SERIAL_PORT_MONITOR.print(" != ");
    SERIAL_PORT_MONITOR.println(b);
    return false;
}

test(ClimateSystemControlFrameTest, Init) {
    ClimateSystemControlFrame c;
    Frame expect(0x540, 0, {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, Ready) {
    ClimateSystemControlFrame c(true);
    Frame expect(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});

    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, InitThenReady) {
    ClimateSystemControlFrame c;

    Frame expect(0x540, 0, {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkFrame(c, expect));

    c.ready();
    expect.data({0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, TurnOff) {
    ClimateSystemControlFrame c(true);

    c.turnOff();
    Frame expect(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x84, 0x00});
    assertTrue(checkFrame(c, expect));

    c.turnOff();
    expect.data({0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, ToggleAuto) {
    ClimateSystemControlFrame c(true);

    c.toggleAuto();
    Frame expect(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00});
    assertTrue(checkFrame(c, expect));

    c.toggleAuto();
    expect.data({0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, ToggleAC) {
    ClimateSystemControlFrame c(true);

    c.toggleAC();
    Frame expect(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x08, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));

    c.toggleAC();
    expect.data({0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, ToggleDual) {
    ClimateSystemControlFrame c(true);

    c.toggleDual();
    Frame expect(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00});
    assertTrue(checkFrame(c, expect));

    c.toggleDual();
    expect.data({0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, CycleMode) {
    ClimateSystemControlFrame c(true);

    c.cycleMode();
    Frame expect(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00});
    assertTrue(checkFrame(c, expect));

    c.cycleMode();
    expect.data({0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, ToggleDefog) {
    ClimateSystemControlFrame c(true);

    c.toggleDefog();
    Frame expect(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00});
    assertTrue(checkFrame(c, expect));

    c.toggleDefog();
    expect.data({0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, IncDriverTemp) {
    ClimateSystemControlFrame c(true);

    c.incDriverTemp();
    Frame expect(0x540, 0, {0x60, 0x40, 0x00, 0x01, 0x00, 0x20, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));

    c.incDriverTemp();
    expect.data({0x60, 0x40, 0x00, 0x02, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, DecDriverTemp) {
    ClimateSystemControlFrame c(true);

    c.decDriverTemp();
    Frame expect(0x540, 0, {0x60, 0x40, 0x00, 0xFF, 0x00, 0x20, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));

    c.decDriverTemp();
    expect.data({0x60, 0x40, 0x00, 0xFE, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, IncPassengerTemp) {
    ClimateSystemControlFrame c(true);

    c.incPassengerTemp();
    Frame expect(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x01, 0x20, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));

    c.incPassengerTemp();
    expect.data({0x60, 0x40, 0x00, 0x00, 0x02, 0x00, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateSystemControlFrameTest, DecPassengerTemp) {
    ClimateSystemControlFrame c(true);

    c.decPassengerTemp();
    Frame expect(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0xFF, 0x20, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));

    c.decPassengerTemp();
    expect.data({0x60, 0x40, 0x00, 0x00, 0xFE, 0x00, 0x04, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateFanControlFrameTest, Init) {
    ClimateFanControlFrame c;
    Frame expect(0x541, 0, {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    assertTrue(checkFrame(c, expect));
}

test(ClimateFanControlFrameTest, Ready) {
    ClimateFanControlFrame c(true);

    Frame expect(0x541, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateFanControlFrameTest, InitThenReady) {
    ClimateFanControlFrame c;

    Frame expect(0x541, 0, {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkFrame(c, expect));

    c.ready();
    expect.data({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateFanControlFrameTest, ToggleRecirculate) {
    ClimateFanControlFrame c(true);

    c.toggleRecirculate();
    Frame expect(0x541, 0, {0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkFrame(c, expect));

    c.toggleRecirculate();
    expect.data({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateFanControlFrameTest, IncFanSpeed) {
    ClimateFanControlFrame c(true);

    c.incFanSpeed();
    Frame expect(0x541, 0, {0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkFrame(c, expect));

    c.incFanSpeed();
    expect.data({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkFrame(c, expect));
}

test(ClimateFanControlFrameTest, DecFanSpeed) {
    ClimateFanControlFrame c(true);

    c.decFanSpeed();
    Frame expect(0x541, 0, {0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkFrame(c, expect));

    c.decFanSpeed();
    expect.data({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
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
