#include <AUnit.h>
#include <Arduino.h>
#include <Controls.h>
#include <Controls/Controls.h>
#include <Faker.h>
#include <Test.h>

namespace R51 {

using namespace aunit;
using ::Faker::FakeClock;

test(ControlsTest, RepeatButtonShortPress) {
    FakeClock clock;
    RepeatButton button(500, &clock);

    button.press();
    clock.delay(100);
    assertFalse(button.trigger());
    assertTrue(button.release());
}

test(ControlsTest, RepeatButtonLongPress) {
    FakeClock clock;
    RepeatButton button(500, &clock);

    button.press();
    clock.delay(100);
    assertFalse(button.trigger());
    clock.delay(500);
    assertTrue(button.trigger());
    clock.delay(500);
    assertTrue(button.trigger());
    clock.delay(101);
    assertFalse(button.release());
}

test(ControlsTest, LongPressButtonShortPress) {
    FakeClock clock;
    LongPressButton button(500, &clock);

    button.press();
    clock.delay(100);
    assertFalse(button.trigger());
    assertTrue(button.release());
}

test(ControlsTest, LongPressButtonLongPress) {
    FakeClock clock;
    LongPressButton button(500, &clock);

    button.press();
    clock.delay(100);
    assertFalse(button.trigger());
    clock.delay(500);
    assertTrue(button.trigger());
    clock.delay(500);
    assertFalse(button.trigger());
    clock.delay(100);
    assertFalse(button.release());
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
