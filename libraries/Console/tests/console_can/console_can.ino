#include <AUnit.h>
#include <Arduino.h>
#include <Canny.h>
#include <Common.h>
#include <Console.h>
#include <Faker.h>
#include <Test.h>

namespace R51 {

using namespace aunit;
using ::Canny::Frame;
using ::Faker::FakeReadStream;
using ::Faker::FakeWriteStream;

byte buffer[64];
const size_t buffer_size = 64;

test(ConsoleCANTest, WriteEmpty) {
    memset(buffer, 0, buffer_size);
    FakeWriteStream stream;
    stream.set(buffer, buffer_size);
    Console console(&stream);

    Message msg;
    console.handle(msg);

    // Ensure nothing was written.
    assertEqual(stream.remaining(), buffer_size);
}

test(ConsoleCANTest, WriteStdFrame) {
    memset(buffer, 0, buffer_size);
    FakeWriteStream stream;
    stream.set(buffer, buffer_size);
    Console console(&stream);

    Frame frame(0x0324, 0, {0x11, 0x22, 0x33, 0x44});
    console.handle(frame);

    assertStringsEqual("recv -324#11:22:33:44\r\n", buffer);
}

test(ConsoleCANTest, WriteExtFrame) {
    memset(buffer, 0, buffer_size);
    FakeWriteStream stream;
    stream.set(buffer, buffer_size);
    Console console(&stream);

    Frame frame(0x45324, 1, {0x11, 0x22, 0x33, 0x44});
    console.handle(frame);

    assertStringsEqual("recv +45324#11:22:33:44\r\n", buffer);
}

test(ConsoleCANTest, ReadStdFrame) {
    Frame expect(0x321, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    strcpy((char*)buffer, "can send -321#11:22:33:44:55:66:77:88\n");

    FakeReadStream stream;
    stream.set(buffer, strlen((char*)buffer));
    FakeYield yield;
    Console console(&stream);

    console.emit(yield);
    assertSize(yield, 1);
    assertEqual(yield.messages()[0].type(), Message::CAN_FRAME);
    assertPrintablesEqual(yield.messages()[0].can_frame(), expect);
}

test(ConsoleCANTest, ReadExtFrame) {
    Frame expect(0x321, 1, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    strcpy((char*)buffer, "can send +321#11:22:33:44:55:66:77:88\n");

    FakeReadStream stream;
    stream.set(buffer, strlen((char*)buffer));
    FakeYield yield;
    Console console(&stream);

    console.emit(yield);
    assertSize(yield, 1);
    assertEqual(yield.messages()[0].type(), Message::CAN_FRAME);
    assertPrintablesEqual(yield.messages()[0].can_frame(), expect);
}

test(ConsoleCANTest, ReadAutoStdFrame) {
    Frame expect(0x321, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    strcpy((char*)buffer, "can send 321#11:22:33:44:55:66:77:88\n");

    FakeReadStream stream;
    stream.set(buffer, strlen((char*)buffer));
    FakeYield yield;
    Console console(&stream);

    console.emit(yield);
    assertSize(yield, 1);
    assertEqual(yield.messages()[0].type(), Message::CAN_FRAME);
    assertPrintablesEqual(yield.messages()[0].can_frame(), expect);
}

test(ConsoleCANTest, ReadAutoExtFrame) {
    Frame expect(0x0FFF, 1, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    strcpy((char*)buffer, "can send 0FFF#11:22:33:44:55:66:77:88\n");

    FakeReadStream stream;
    stream.set(buffer, strlen((char*)buffer));
    FakeYield yield;
    Console console(&stream);

    console.emit(yield);
    assertSize(yield, 1);
    assertEqual(yield.messages()[0].type(), Message::CAN_FRAME);
    assertPrintablesEqual(yield.messages()[0].can_frame(), expect);
}

test(ConsoleCANTest, ReadNoColons) {
    Frame expect(0x321, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    strcpy((char*)buffer, "can send -321#1122334455667788\n");

    FakeReadStream stream;
    stream.set(buffer, strlen((char*)buffer));
    FakeYield yield;
    Console console(&stream);

    console.emit(yield);
    assertSize(yield, 1);
    assertEqual(yield.messages()[0].type(), Message::CAN_FRAME);
    assertPrintablesEqual(yield.messages()[0].can_frame(), expect);
}

test(ConsoleFrameTest, ReadValidButBadlyFormed) {
    Frame expect(0x456, 0, {0x11, 0x02, 0xFF, 0xAB, 0x0C, 0x0E});
    strcpy((char*)buffer, "can send 00000456#112:FF:ABC:E\n");

    FakeReadStream stream;
    stream.set(buffer, strlen((char*)buffer));
    FakeYield yield;
    Console console(&stream);

    console.emit(yield);
    assertSize(yield, 1);
    assertEqual(yield.messages()[0].type(), Message::CAN_FRAME);
    assertPrintablesEqual(yield.messages()[0].can_frame(), expect);
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
