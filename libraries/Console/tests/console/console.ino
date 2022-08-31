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

test(ConsoleTest, WriteEmpty) {
    memset(buffer, 0, buffer_size);
    FakeWriteStream stream;
    stream.set(buffer, buffer_size);
    Console monitor(&stream);

    Message msg;
    monitor.handle(msg);

    // Ensure nothing was written.
    assertEqual(stream.remaining(), buffer_size);
}

test(ConsoleTest, WriteEventEmptyPayload) {
    memset(buffer, 0, buffer_size);
    FakeWriteStream stream;
    stream.set(buffer, buffer_size);
    Console monitor(&stream);

    Event event(0x01, 0x02);
    monitor.handle(event);

    assertStringsEqual("recv 01:02#FF:FF:FF:FF:FF:FF\r\n", buffer);
}

test(ConsoleTest, WriteEventPartialPayload) {
    memset(buffer, 0, buffer_size);
    FakeWriteStream stream;
    stream.set(buffer, buffer_size);
    Console monitor(&stream);

    Event event(0x01, 0x02, {0xAA, 0xBB});
    monitor.handle(event);

    assertStringsEqual("recv 01:02#AA:BB:FF:FF:FF:FF\r\n", buffer);
}

test(ConsoleTest, WriteEventFullPayload) {
    memset(buffer, 0, buffer_size);
    FakeWriteStream stream;
    stream.set(buffer, buffer_size);
    Console monitor(&stream);

    Event event(0x01, 0x02, {0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE});
    monitor.handle(event);

    assertStringsEqual("recv 01:02#99:AA:BB:CC:DD:EE\r\n", buffer);
}

test(ConsoleTest, WriteStdFrame) {
    memset(buffer, 0, buffer_size);
    FakeWriteStream stream;
    stream.set(buffer, buffer_size);
    Console monitor(&stream);

    Frame frame(0x0324, 0, {0x11, 0x22, 0x33, 0x44});
    monitor.handle(frame);

    assertStringsEqual("recv -324#11:22:33:44\r\n", buffer);
}

test(ConsoleTest, WriteExtFrame) {
    memset(buffer, 0, buffer_size);
    FakeWriteStream stream;
    stream.set(buffer, buffer_size);
    Console monitor(&stream);

    Frame frame(0x45324, 1, {0x11, 0x22, 0x33, 0x44});
    monitor.handle(frame);

    assertStringsEqual("recv +45324#11:22:33:44\r\n", buffer);
}

test(ConsoleTest, ReadEmptyEvent) {
    Event expect(0x03, 0x04);
    strcpy((char*)buffer, "03:04\n");

    FakeReadStream stream;
    stream.set(buffer, strlen((char*)buffer));
    FakeYield yield;
    Console monitor(&stream);

    monitor.emit(yield);
    assertSize(yield, 1);
    assertEqual(yield.messages()[0].type(), Message::EVENT);
    assertPrintablesEqual(yield.messages()[0].event(), expect);
}

test(ConsoleTest, ReadPartialEvent) {
    Event expect(0x03, 0x04, {0x22, 0x33});
    strcpy((char*)buffer, "03:04#22:33\r\n");

    FakeReadStream stream;
    stream.set(buffer, strlen((char*)buffer));
    FakeYield yield;
    Console monitor(&stream);

    monitor.emit(yield);
    assertSize(yield, 1);
    assertEqual(yield.messages()[0].type(), Message::EVENT);
    assertPrintablesEqual(yield.messages()[0].event(), expect);
}

test(ConsoleTest, ReadFullEvent) {
    Event expect(0x03, 0x04, {0x22, 0x33, 0x44, 0x55, 0x66, 0x77});
    strcpy((char*)buffer, "03:04#22:33:44:55:66:77\n");

    FakeReadStream stream;
    stream.set(buffer, strlen((char*)buffer));
    FakeYield yield;
    Console monitor(&stream);

    monitor.emit(yield);
    assertSize(yield, 1);
    assertEqual(yield.messages()[0].type(), Message::EVENT);
    assertPrintablesEqual(yield.messages()[0].event(), expect);
}

test(ConsoleTest, ReadNoColons) {
    Event expect(0x03, 0x04, {0x22, 0x33, 0x44, 0x55, 0x66, 0x77});
    strcpy((char*)buffer, "03:04#223344556677\n");

    FakeReadStream stream;
    stream.set(buffer, strlen((char*)buffer));
    FakeYield yield;
    Console monitor(&stream);

    monitor.emit(yield);
    assertSize(yield, 1);
    assertEqual(yield.messages()[0].type(), Message::EVENT);
    assertPrintablesEqual(yield.messages()[0].event(), expect);
}

test(ConsoleTest, ReadValidButBadlyFormedEvent) {
    Event expect(0x03, 0x04, {0x22, 0x33, 0x04});
    strcpy((char*)buffer, "03:04#2233:4\n");

    FakeReadStream stream;
    stream.set(buffer, strlen((char*)buffer));
    FakeYield yield;
    Console monitor(&stream);

    monitor.emit(yield);
    assertSize(yield, 1);
    assertEqual(yield.messages()[0].type(), Message::EVENT);
    assertPrintablesEqual(yield.messages()[0].event(), expect);
}

test(ConsoleTest, ReadInvalidEvent) {
    strcpy((char*)buffer, "0304#\n");

    FakeReadStream stream;
    stream.set(buffer, strlen((char*)buffer));
    FakeYield yield;
    Console monitor(&stream);

    monitor.emit(yield);
    assertSize(yield, 0);
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
