#include <AUnit.h>
#include <Arduino.h>
#include <Canny.h>
#include <Common.h>
#include <Console.h>
#include <Faker.h>
#include <Test.h>

namespace R51 {

using namespace aunit;
using ::Faker::FakeReadStream;
using ::Faker::FakeWriteStream;

byte buffer[64];
const size_t buffer_size = 64;

test(ConsoleEventTest, WriteEmpty) {
    FakeYield yield;
    memset(buffer, 0, buffer_size);
    FakeWriteStream stream;
    stream.set(buffer, buffer_size);
    ConsoleNode console(&stream);

    Message msg;
    console.handle(msg, yield);
    assertSize(yield, 0);

    // Ensure nothing was written.
    assertEqual(stream.remaining(), buffer_size);
}

test(ConsoleEventTest, WriteEventEmptyPayload) {
    FakeYield yield;
    memset(buffer, 0, buffer_size);
    FakeWriteStream stream;
    stream.set(buffer, buffer_size);
    ConsoleNode console(&stream);

    Event event(0x01, 0x02);
    console.handle(event, yield);
    assertSize(yield, 0);

    assertStringsEqual("console: event recv 01:02#FF:FF:FF:FF:FF:FF\r\n", buffer);
}

test(ConsoleEventTest, WriteEventPartialPayload) {
    FakeYield yield;
    memset(buffer, 0, buffer_size);
    FakeWriteStream stream;
    stream.set(buffer, buffer_size);
    ConsoleNode console(&stream);

    Event event(0x01, 0x02, {0xAA, 0xBB});
    console.handle(event, yield);
    assertSize(yield, 0);

    assertStringsEqual("console: event recv 01:02#AA:BB:FF:FF:FF:FF\r\n", buffer);
}

test(ConsoleEventTest, WriteEventFullPayload) {
    FakeYield yield;
    memset(buffer, 0, buffer_size);
    FakeWriteStream stream;
    stream.set(buffer, buffer_size);
    ConsoleNode console(&stream);

    Event event(0x01, 0x02, {0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE});
    console.handle(event, yield);
    assertSize(yield, 0);

    assertStringsEqual("console: event recv 01:02#99:AA:BB:CC:DD:EE\r\n", buffer);
}

test(ConsoleEventTest, ReadEmptyEvent) {
    Event expect(0x03, 0x04);
    strcpy((char*)buffer, "event send 03:04\n");

    FakeReadStream stream;
    stream.set(buffer, strlen((char*)buffer));
    FakeYield yield;
    ConsoleNode console(&stream);

    console.emit(yield);
    assertSize(yield, 1);
    assertEqual(yield.messages()[0].type(), Message::EVENT);
    assertPrintablesEqual(*yield.messages()[0].event(), expect);
}

test(ConsoleEventTest, ReadPartialEvent) {
    Event expect(0x03, 0x04, {0x22, 0x33});
    strcpy((char*)buffer, "event send 03:04#22:33\r\n");

    FakeReadStream stream;
    stream.set(buffer, strlen((char*)buffer));
    FakeYield yield;
    ConsoleNode console(&stream);

    console.emit(yield);
    assertSize(yield, 1);
    assertEqual(yield.messages()[0].type(), Message::EVENT);
    assertPrintablesEqual(*yield.messages()[0].event(), expect);
}

test(ConsoleEventTest, ReadFullEvent) {
    Event expect(0x03, 0x04, {0x22, 0x33, 0x44, 0x55, 0x66, 0x77});
    strcpy((char*)buffer, "event send 03:04#22:33:44:55:66:77\n");

    FakeReadStream stream;
    stream.set(buffer, strlen((char*)buffer));
    FakeYield yield;
    ConsoleNode console(&stream);

    console.emit(yield);
    assertSize(yield, 1);
    assertEqual(yield.messages()[0].type(), Message::EVENT);
    assertPrintablesEqual(*yield.messages()[0].event(), expect);
}

test(ConsoleEventTest, ReadNoColons) {
    Event expect(0x03, 0x04, {0x22, 0x33, 0x44, 0x55, 0x66, 0x77});
    strcpy((char*)buffer, "event send 03:04#223344556677\n");

    FakeReadStream stream;
    stream.set(buffer, strlen((char*)buffer));
    FakeYield yield;
    ConsoleNode console(&stream);

    console.emit(yield);
    assertSize(yield, 1);
    assertEqual(yield.messages()[0].type(), Message::EVENT);
    assertPrintablesEqual(*yield.messages()[0].event(), expect);
}

test(ConsoleEventTest, ReadValidButBadlyFormedEvent) {
    Event expect(0x03, 0x04, {0x22, 0x33, 0x04});
    strcpy((char*)buffer, "event send 03:04#2233:4\n");

    FakeReadStream stream;
    stream.set(buffer, strlen((char*)buffer));
    FakeYield yield;
    ConsoleNode console(&stream);

    console.emit(yield);
    assertSize(yield, 1);
    assertEqual(yield.messages()[0].type(), Message::EVENT);
    assertPrintablesEqual(*yield.messages()[0].event(), expect);
}

test(ConsoleEventTest, ReadInvalidEvent) {
    strcpy((char*)buffer, "0304#\n");

    FakeReadStream stream;
    stream.set(buffer, strlen((char*)buffer));
    FakeYield yield;
    ConsoleNode console(&stream);

    console.emit(yield);
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
