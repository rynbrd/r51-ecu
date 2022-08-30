#include <AUnit.h>
#include <Arduino.h>
#include <Canny.h>
#include <Common.h>

namespace R51 {

using namespace aunit;
using ::Canny::Frame;

test(MessageTest, Event) {
    Event event(0x01, 0x02);
    Message msg(event);
    assertEqual(&event, &msg.event());
}

test(MessageTest, CANFrame) {
    Frame frame(0x123, 4, {0x01, 0x02, 0x03, 0x04});
    Message msg(frame);
    assertEqual(&frame, &msg.can_frame());
}

test(MessageTest, EmptyEquals) {
    Message left;
    Message right;
    assertTrue(left == right);
}

test(MessageTest, SameMessageEquals) {
    Frame frame(0x123, 4, {0x01, 0x02, 0x03, 0x04});
    Message msg(frame);
    assertTrue(msg == msg);
}

test(MessageTest, SamePayloadEquals) {
    Frame frame(0x123, 4, {0x01, 0x02, 0x03, 0x04});
    Message left(frame);
    Message right(frame);
    assertTrue(left == right);
}

test(MessageTest, DifferentPayloadsNotEqual) {
    Frame frame1(0x123, 4, {0x01, 0x02, 0x03, 0x04});
    Frame frame2(0x123, 4, {0x01, 0x02, 0x03, 0x04});
    Message left(frame1);
    Message right(frame2);
    assertTrue(!(left == right));
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
