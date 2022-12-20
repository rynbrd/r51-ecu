#include <AUnit.h>
#include <Arduino.h>
#include <Canny.h>
#include <Core.h>

namespace R51 {

using namespace aunit;

enum class TestEvent : uint8_t {
    EVENT_1 = 0x01,
    EVENT_2 = 0x02,
};

test(EventTest, Empty) {
    uint8_t expect[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    Event event;
    assertEqual(event.subsystem, 0);
    assertEqual(event.id, 0);
    assertEqual(memcmp(expect, event.data, 6), 0);
}

test(EventTest, WithID) {
    uint8_t expect[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    Event event(0x01, 0xA1);
    assertEqual(event.subsystem, 0x01);
    assertEqual(event.id, 0xA1);
    assertEqual(memcmp(expect, event.data, 6), 0);
}

test(EventTest, WithData) {
    uint8_t expect[] = {0x00, 0x11, 0xFF, 0xFF, 0xFF, 0xFF};
    Event event(0x02, 0xA2, (uint8_t[]){0x00, 0x11});
    assertEqual(event.subsystem, 0x02);
    assertEqual(event.id, 0xA2);
    assertEqual(memcmp(expect, event.data, 6), 0);
}

test(EventTest, EnumsAndData) {
    uint8_t expect[] = {0x00, 0x11, 0xFF, 0xFF, 0xFF, 0xFF};
    Event event(SubSystem::ECM, (uint8_t)TestEvent::EVENT_2, (uint8_t[]){0x00, 0x11});
    assertTrue((SubSystem)event.subsystem == SubSystem::ECM);
    assertTrue((TestEvent)event.id == TestEvent::EVENT_2);
    assertEqual(memcmp(expect, event.data, 6), 0);
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
