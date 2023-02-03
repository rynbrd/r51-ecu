#include <AUnit.h>
#include <Arduino.h>
#include <Controls.h>
#include <Core.h>
#include <Faker.h>
#include <Test.h>
#include <Vehicle.h>

namespace R51 {

using namespace aunit;
using ::Faker::FakeClock;

class SteeringTest : public TestOnce {
    public:
        void shortPress(SteeringKey key, const Event& expect) {
            FakeClock clock;
            SteeringControls steering(1, &clock);
            KeyState event(1, (uint8_t)key);
            FakeYield yield;

            // press once
            event.pressed(true);
            steering.handle(MessageView(&event), yield);
            steering.emit(yield);
            assertSize(yield, 0);

            clock.delay(100);
            steering.emit(yield);
            assertSize(yield, 0);

            event.pressed(false);
            steering.handle(MessageView(&event), yield);
            steering.emit(yield);
            assertSize(yield, 1);
            assertIsEvent(yield.messages()[0], expect);
            yield.clear();

            clock.delay(10000);

            // press again
            event.pressed(true);
            steering.handle(MessageView(&event), yield);
            steering.emit(yield);
            assertSize(yield, 0);

            clock.delay(100);
            steering.emit(yield);
            assertSize(yield, 0);

            event.pressed(false);
            steering.handle(MessageView(&event), yield);
            steering.emit(yield);
            assertSize(yield, 1);
            assertIsEvent(yield.messages()[0], expect);
        }

        void longPress(SteeringKey key, const Event& expect_short, const Event& expect_long) {
            FakeClock clock;
            SteeringControls steering(1, &clock);
            KeyState event(1, (uint8_t)key);
            FakeYield yield;

            // long press once
            event.pressed(true);
            steering.handle(MessageView(&event), yield);
            steering.emit(yield);
            assertSize(yield, 0);

            clock.delay(100);
            steering.emit(yield);
            assertSize(yield, 0);

            clock.delay(3000);
            steering.emit(yield);
            assertSize(yield, 1);
            assertIsEvent(yield.messages()[0], expect_long);
            yield.clear();

            clock.delay(300);
            event.pressed(false);
            steering.handle(MessageView(&event), yield);
            steering.emit(yield);
            assertSize(yield, 0);

            clock.delay(10000);

            // short press
            event.pressed(true);
            steering.handle(MessageView(&event), yield);
            steering.emit(yield);
            assertSize(yield, 0);

            clock.delay(100);
            steering.emit(yield);
            assertSize(yield, 0);

            event.pressed(false);
            steering.handle(MessageView(&event), yield);
            steering.emit(yield);
            assertSize(yield, 1);
            assertIsEvent(yield.messages()[0], expect_short);
        }

        void repeatPress(SteeringKey key, const Event& expect) {
            FakeClock clock;
            SteeringControls steering(1, &clock);
            KeyState event(1, (uint8_t)key);
            FakeYield yield;

            // short press
            event.pressed(true);
            steering.handle(MessageView(&event), yield);
            steering.emit(yield);
            assertSize(yield, 0);

            clock.delay(100);
            steering.emit(yield);
            assertSize(yield, 0);

            event.pressed(false);
            steering.handle(MessageView(&event), yield);
            steering.emit(yield);
            assertSize(yield, 1);
            assertIsEvent(yield.messages()[0], expect);
            yield.clear();

            clock.delay(10000);

            // press and hold
            event.pressed(true);
            steering.handle(MessageView(&event), yield);
            steering.emit(yield);
            assertSize(yield, 0);

            clock.delay(100);
            steering.emit(yield);
            assertSize(yield, 0);

            clock.delay(500);
            steering.emit(yield);
            assertSize(yield, 1);
            assertIsEvent(yield.messages()[0], expect);
            yield.clear();

            clock.delay(500);
            steering.emit(yield);
            assertSize(yield, 1);
            assertIsEvent(yield.messages()[0], expect);
            yield.clear();

            event.pressed(false);
            steering.handle(MessageView(&event), yield);
            steering.emit(yield);
            assertSize(yield, 0);

            clock.delay(10000);

            // short press again
            event.pressed(true);
            steering.handle(MessageView(&event), yield);
            steering.emit(yield);
            assertSize(yield, 0);

            clock.delay(100);
            steering.emit(yield);
            assertSize(yield, 0);

            event.pressed(false);
            steering.handle(MessageView(&event), yield);
            steering.emit(yield);
            assertSize(yield, 1);
            assertIsEvent(yield.messages()[0], expect);
            yield.clear();
        }
};

testF(SteeringTest, PowerShortPress) {
    shortPress(SteeringKey::POWER,
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::PLAYBACK_TOGGLE_CMD));
}

testF(SteeringTest, PowerLongPress) {
    longPress(SteeringKey::POWER,
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::PLAYBACK_TOGGLE_CMD),
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::POWER_TOGGLE_CMD));
}

testF(SteeringTest, ModeShortPress) {
    shortPress(SteeringKey::MODE,
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SOURCE_NEXT_CMD));
}

testF(SteeringTest, VolumeUpShortPress) {
    shortPress(SteeringKey::VOLUME_UP,
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::VOLUME_INC_CMD));
}

testF(SteeringTest, VolumeUpLongPress) {
    repeatPress(SteeringKey::VOLUME_UP,
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::VOLUME_INC_CMD));
}

testF(SteeringTest, VolumeDownShortPress) {
    shortPress(SteeringKey::VOLUME_DOWN,
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::VOLUME_DEC_CMD));
}

testF(SteeringTest, VolumeDownLongPress) {
    repeatPress(SteeringKey::VOLUME_DOWN,
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::VOLUME_DEC_CMD));
}

testF(SteeringTest, SeekUpShortPress) {
    shortPress(SteeringKey::SEEK_UP,
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::PLAYBACK_PREV_CMD));
}

testF(SteeringTest, SeekUpLongPress) {
    repeatPress(SteeringKey::SEEK_UP,
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::PLAYBACK_PREV_CMD));
}

testF(SteeringTest, SeekDownShortPress) {
    shortPress(SteeringKey::SEEK_DOWN,
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::PLAYBACK_NEXT_CMD));
}

testF(SteeringTest, SeekDownLongPress) {
    repeatPress(SteeringKey::SEEK_DOWN,
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::PLAYBACK_NEXT_CMD));
}

}

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
