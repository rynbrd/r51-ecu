#include <AUnit.h>
#include <Arduino.h>
#include <Faker.h>
#include <Test.h>
#include <Vehicle.h>

namespace R51 {

using namespace aunit;
using ::Faker::FakeClock;
using ::Faker::FakeGPIO;

class SteeringKeypadTest : public TestOnce {
    public:
        static constexpr const int values[] = {22, 332, 1012};
        static const int pin_a = 1;
        static const int pin_b = 2;
        static const uint8_t keypad_id = 0x10;

        void assertButtonPress(uint32_t pin, uint32_t value, SteeringKey key) {
            FakeClock clock;
            FakeGPIO gpio;
            FakeYield yield;

            KeyState released(keypad_id, (uint8_t)key, false);
            KeyState pressed(keypad_id, (uint8_t)key, true);

            // initialize keypad
            SteeringKeypad keypad(keypad_id, pin_a, pin_b, &clock, &gpio);
            keypad.begin();
            gpio.analogWrite(pin_a, 4000);
            gpio.analogWrite(pin_b, 4000);

            // ensure button not yet pressed
            keypad.emit(yield);
            assertSize(yield, 0);
            yield.clear();

            // set analog value, check button, wait for debounce, check button again
            gpio.analogWrite(pin, value);
            keypad.emit(yield);
            assertSize(yield, 0);
            clock.delay(100);
            keypad.emit(yield);
            assertSize(yield, 1);
            assertIsEvent(yield.messages()[0], pressed);
            yield.clear();

            // release the button and advance time past the debounce
            gpio.analogWrite(pin, 1023);
            keypad.emit(yield);
            assertSize(yield, 0);
            clock.delay(100);
            keypad.emit(yield);
            assertSize(yield, 1);
            assertIsEvent(yield.messages()[0], released);
        }
};

testF(SteeringKeypadTest, Power) {
    assertButtonPress(pin_a, values[0], SteeringKey::POWER);
}

testF(SteeringKeypadTest, SeekDown) {
    assertButtonPress(pin_a, values[1], SteeringKey::SEEK_UP);
}

testF(SteeringKeypadTest, VolumeDown) {
    assertButtonPress(pin_a, values[2], SteeringKey::VOLUME_UP);
}

testF(SteeringKeypadTest, Mode) {
    assertButtonPress(pin_b, values[0], SteeringKey::MODE);
}

testF(SteeringKeypadTest, SeekUp) {
    assertButtonPress(pin_b, values[1], SteeringKey::SEEK_DOWN);
}

testF(SteeringKeypadTest, VolumeUp) {
    assertButtonPress(pin_b, values[2], SteeringKey::VOLUME_DOWN);
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
