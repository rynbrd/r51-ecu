#include <AUnit.h>
#include <Arduino.h>
#include <Canny.h>
#include <Common.h>
#include <Console.h>
#include <Faker.h>
#include <Test.h>

namespace R51 {

using namespace aunit;
using ::Canny::J1939Message;
using ::Faker::FakeReadStream;
using ::Faker::FakeWriteStream;

byte buffer[64];
const size_t buffer_size = 64;

test(ConsoleJ1939Test, WriteUnmuted) {
    memset(buffer, 0, buffer_size);
    FakeWriteStream stream;
    stream.set(buffer, buffer_size);
    ConsoleNode console(&stream);

    J1939Message msg(0xEF00, 0x31, 0x42, 0x01);
    msg.data({0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    console.handle(msg);

    assertStringsEqual("console: j1939 recv 4EF4231#11:22:33:44:55:66:77:88\r\n", buffer);
}

test(ConsoleJ1939Test, WriteMuted) {
    memset(buffer, 0, buffer_size);
    FakeWriteStream stream;
    stream.set(buffer, buffer_size);
    ConsoleNode console(&stream);
    console.console()->j1939_mute(true);

    J1939Message msg(0xEF00, 0x31, 0x42, 0x01);
    msg.data({0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    console.handle(msg);

    assertStringsEqual("", buffer);
}

test(ConsoleJ1939Test, ReadMessage) {
    J1939Message expect(0xEF00, 0x31, 0x42, 0x01);
    expect.data({0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    strcpy((char*)buffer, "j1939 send 04EF4231#11:22:33:44:55:66:77:88\n");

    FakeReadStream stream;
    stream.set(buffer, strlen((char*)buffer));
    FakeYield yield;
    ConsoleNode console(&stream);

    console.emit(yield);
    assertSize(yield, 1);
    assertEqual(yield.messages()[0].type(), Message::J1939_MESSAGE);
    assertPrintablesEqual(yield.messages()[0].j1939_message(), expect);
}

test(ConsoleJ1939Test, SetMute) {

    FakeReadStream stream;
    FakeYield yield;

    ConsoleNode console(&stream);
    assertFalse(console.console()->j1939_mute());

    strcpy((char*)buffer, "j1939 mute\n");
    stream.set(buffer, strlen((char*)buffer));
    console.emit(yield);
    assertTrue(console.console()->j1939_mute());
    assertSize(yield, 0);

    strcpy((char*)buffer, "j1939 unmute\n");
    stream.set(buffer, strlen((char*)buffer));
    console.emit(yield);
    assertFalse(console.console()->j1939_mute());
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
