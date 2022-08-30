#include <AUnit.h>
#include <Arduino.h>
#include <R51Core.h>
#include <Faker.h>

namespace R51 {

using namespace aunit;
using ::Faker::FakeClock;

test(TickerTest, TickAndReset) {
    FakeClock clock;
    Ticker t(500, &clock);

    assertFalse(t.active());

    clock.set(200);
    assertFalse(t.active());

    clock.set(500);
    assertTrue(t.active());
    clock.set(501);
    assertTrue(t.active());
    t.reset();

    clock.set(502);
    assertFalse(t.active());

    clock.set(1000);
    assertFalse(t.active());
    clock.set(1001);
    assertTrue(t.active());
}

test(TickerTest, EarlyReset) {
    FakeClock clock;
    Ticker t(500, &clock);
    assertFalse(t.active());

    clock.set(200);
    assertFalse(t.active());

    t.reset();
    assertFalse(t.active());

    clock.set(500);
    assertFalse(t.active());

    clock.set(700);
    assertTrue(t.active());
}

test(TickerTest, TickAndResetNewInterval) {
    FakeClock clock;
    Ticker t(500, &clock);

    assertFalse(t.active());

    clock.set(500);
    assertTrue(t.active());
    t.reset(1000);

    clock.set(1000);
    assertFalse(t.active());

    clock.set(1500);
    assertTrue(t.active());
}

test(TickerTest, NoTick) {
    FakeClock clock;
    Ticker t(0, &clock);

    assertFalse(t.active());
    clock.set(1000000000);
    assertFalse(t.active());
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
