#include <AUnit.h>
#include <Arduino.h>
#include <Canny.h>
#include <Core.h>

namespace R51 {

using namespace aunit;
using ::Canny::CAN20Frame;
using ::Canny::J1939Message;

test(MessageValueTest, Event) {
    Event event(0x01, 0x02);
    MessageValue msg(event);
    assertEqual(msg.type(), Message::EVENT);
    assertTrue(event == *msg.event());
    assertNotEqual(&event, msg.event());
}

test(MessageValueTest, CopyEventValue) {
    Event event(0x01, 0x02);
    MessageValue msg(event);

    MessageValue copy(msg);
    assertTrue(msg == copy);
    assertTrue(event == *copy.event());
}

test(MessageValueTest, CopyEventView) {
    Event event(0x01, 0x02);
    MessageView msg(&event);

    MessageValue copy(msg);
    assertTrue(msg == copy);
    assertTrue(&event != copy.event());
    assertTrue(event == *copy.event());
}

test(MessageValueTest, CANFrame) {
    CAN20Frame frame(0x123, 4, {0x01, 0x02, 0x03, 0x04});
    MessageValue msg(frame);

    assertEqual(msg.type(), Message::CAN_FRAME);
    assertTrue(frame == *msg.can_frame());
    assertNotEqual(&frame, msg.can_frame());
}

test(MessageValueTest, CopyCANFrameValue) {
    CAN20Frame frame(0x123, 4, {0x01, 0x02, 0x03, 0x04});
    MessageValue msg(frame);
    MessageValue copy(msg);

    assertTrue(msg == copy);
    assertTrue(frame == *copy.can_frame());
}

test(MessageValueTest, CopyCANFrameView) {
    CAN20Frame frame(0x123, 4, {0x01, 0x02, 0x03, 0x04});
    MessageView msg(&frame);
    MessageValue copy(msg);

    assertTrue(msg == copy);
    assertTrue(&frame != copy.can_frame());
    assertTrue(frame == *copy.can_frame());
}

test(MessageValueTest, J1939Message) {
    J1939Message j1939(0xFF00, 0x10);
    MessageValue msg(j1939);
    assertEqual(msg.type(), Message::J1939_MESSAGE);
    assertNotEqual(&j1939, msg.j1939_message());
}

test(MessageValueTest, CopyJ1939MessageValue) {
    J1939Message j1939(0xFF00, 0x10);
    MessageValue msg(j1939);
    MessageValue copy(msg);

    assertTrue(msg == copy);
    assertTrue(j1939 == *copy.j1939_message());
}

test(MessageValueTest, CopyJ1939MessageView) {
    J1939Message j1939(0xFF00, 0x10);
    MessageView msg(&j1939);
    MessageValue copy(msg);

    assertTrue(msg == copy);
    assertTrue(&j1939 != copy.j1939_message());
    assertTrue(j1939 == *copy.j1939_message());
}

test(MessageValueTest, J1939Claim) {
    J1939Claim claim(0x23, 1234);
    MessageValue msg(claim);
    assertEqual(msg.type(), Message::J1939_CLAIM);
    assertNotEqual(&claim, msg.j1939_claim());
}

test(MessageValueTest, CopyJ1939ClaimValue) {
    J1939Claim claim(0x23, 1234);
    MessageValue msg(claim);
    MessageValue copy(msg);

    assertTrue(msg == copy);
    assertTrue(claim == *copy.j1939_claim());
}

test(MessageValueTest, CopyJ1939ClaimView) {
    J1939Claim claim(0x23, 1234);
    MessageView msg(&claim);
    MessageValue copy(msg);

    assertTrue(msg == copy);
    assertTrue(&claim != copy.j1939_claim());
    assertTrue(claim == *copy.j1939_claim());
}

test(MessageValueTest, EmptyEquals) {
    MessageValue left;
    MessageValue right;
    assertTrue(left == right);
}

test(MessageValueTest, CopyBaseClass) {
    CAN20Frame frame(0x123, 4, {0x01, 0x02, 0x03, 0x04});
    MessageView msg(&frame);
    const Message& ref = msg;
    MessageValue copy(ref);

    assertTrue(msg == copy);
    assertTrue(ref == copy);
    assertTrue(&frame != copy.can_frame());
    assertTrue(frame == *copy.can_frame());
}

test(MessageValueTest, EventNoCrashOnUse) {
    Event event(0x01, 0x02);
    MessageView msg(&event);
    msg.printTo(Serial);Serial.println();
    MessageValue copy(msg);
    copy.printTo(Serial);Serial.println();
}

test(MessageValueTest, FrameNoCrashOnUse) {
    CAN20Frame frame(0x123, 4, {0x01, 0x02, 0x03, 0x04});
    MessageView msg(&frame);
    msg.printTo(Serial);Serial.println();
    MessageValue copy(msg);
    copy.printTo(Serial);Serial.println();
}

test(MessageValueTest, J1939MessageNoCrashOnUse) {
    J1939Message j1939(0xFF00, 0x10);
    MessageView msg(&j1939);
    msg.printTo(Serial);Serial.println();
    MessageValue copy(msg);
    copy.printTo(Serial);Serial.println();
}

test(MessageValueTest, J1939ClaimNoCrashOnUse) {
    J1939Claim claim(0x23, 1234);
    MessageView msg(&claim);
    msg.printTo(Serial);Serial.println();
    MessageValue copy(msg);
    copy.printTo(Serial);Serial.println();
}

test(MessageViewTest, Event) {
    Event event(0x01, 0x02);
    MessageView msg(&event);
    assertEqual(msg.type(), Message::EVENT);
    assertEqual(&event, msg.event());
}

test(MessageViewTest, CANFrame) {
    CAN20Frame frame(0x123, 4, {0x01, 0x02, 0x03, 0x04});
    MessageView msg(&frame);
    assertEqual(msg.type(), Message::CAN_FRAME);
    assertEqual(&frame, msg.can_frame());
}

test(MessageViewTest, J1939Message) {
    J1939Message j1939(0xFF00, 0x10);
    MessageView msg(&j1939);
    assertEqual(msg.type(), Message::J1939_MESSAGE);
    assertEqual(&j1939, msg.j1939_message());
}

test(MessageViewTest, J1939Claim) {
    J1939Claim claim(0x23, 1234);
    MessageView msg(&claim);
    assertEqual(msg.type(), Message::J1939_CLAIM);
    assertEqual(&claim, msg.j1939_claim());
}

test(MessageViewTest, EmptyEquals) {
    MessageView left;
    MessageView right;
    assertTrue(left == right);
}

test(MessageViewTest, SameMessageEquals) {
    CAN20Frame frame(0x123, 4, {0x01, 0x02, 0x03, 0x04});
    MessageView msg(&frame);
    assertTrue(msg == msg);
}

test(MessageViewTest, SamePayloadEquals) {
    CAN20Frame frame(0x123, 4, {0x01, 0x02, 0x03, 0x04});
    MessageView left(&frame);
    MessageView right(&frame);
    assertTrue(left == right);
}

test(MessageViewTest, DifferentPayloadsSameValueEqual) {
    CAN20Frame frame1(0x123, 4, {0x01, 0x02, 0x03, 0x04});
    CAN20Frame frame2(0x123, 4, {0x01, 0x02, 0x03, 0x04});
    MessageView left(&frame1);
    MessageView right(&frame2);
    assertTrue(left == right);
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
