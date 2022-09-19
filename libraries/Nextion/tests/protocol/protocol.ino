#include <AUnit.h>
#include <Arduino.h>
#include <Faker.h>
#include <Nextion.h>

using namespace aunit;

namespace Nextion {

using ::Faker::FakeReadStream;
using ::Faker::FakeWriteStream;

size_t printBuffer(uint8_t* buffer, size_t size) {
    for (size_t i = 0; i < size; i++) {
        Serial.print(buffer[i], HEX);
    }
    return size;
}

size_t printRecv(uint8_t* buffer, size_t size) {
    size_t n = 0;
    n += Serial.print("recv ");
    n += Serial.print(size);
    n += Serial.print(": ");
    n += printBuffer(buffer, size);
    n += Serial.println();
    return n;
}

test(ProtocolTest, RecvNothing) {
    FakeReadStream stream;
    Protocol p(&stream);

    uint8_t expect[] = {};
    stream.set(expect, 0);

    uint8_t actual[7];
    size_t n = p.recv(actual, 7);

    assertEqual(n, (size_t)0);
}

test(ProtocolTest, RecvOne) {
    FakeReadStream stream;
    Protocol p(&stream);

    uint8_t expect[] = {0x65, 0x0B, 0x17, 0x01, 0xFF, 0xFF, 0xFF};
    stream.set(expect, 7);

    uint8_t actual[7];
    size_t n = p.recv(actual, 7);

    assertEqual(n, (size_t)4);
    assertEqual(memcmp(expect, actual, n), 0);
    assertEqual(stream.remaining(), (size_t)0);
}

test(ProtocolTest, RecvMultiple) {
    FakeReadStream stream;
    Protocol p(&stream);

    uint8_t expect[] = {
        0x65, 0x02, 0x12, 0x00, 0xFF, 0xFF, 0xFF,
        0x65, 0x0B, 0x17, 0x01, 0xFF, 0xFF, 0xFF,
    };
    stream.set(expect, 14);

    uint8_t actual[7];
    size_t n = p.recv(actual, 7);
    assertEqual(n, (size_t)4);
    assertEqual(memcmp(expect, actual, n), 0);
    assertEqual(stream.remaining(), (size_t)7);

    n = p.recv(actual, 7);
    assertEqual(n, (size_t)4);
    assertEqual(memcmp(expect+7, actual, n), 0);
    assertEqual(stream.remaining(), (size_t)0);
}

test(ProtocolTest, RecvOverflow) {
    FakeReadStream stream;
    Protocol p(&stream);

    uint8_t expect[] = {0x65, 0x0B, 0x17, 0x01, 0xFF, 0xFF, 0xFF};
    stream.set(expect, 7);

    uint8_t actual[2];
    size_t n = p.recv(actual, 2);

    assertEqual(n, (size_t)0);
    assertEqual(stream.remaining(), (size_t)0);
}

test(ProtocolTest, SendBytes) {
    FakeWriteStream stream;
    Protocol p(&stream);

    uint8_t expect[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x64, 0x70, 0xFF, 0xFF, 0xFF};
    uint8_t actual[32];
    stream.set(actual, 32);

    uint8_t cmd[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x64, 0x70};
    p.send(cmd, 7);
    assertEqual(memcmp(actual, expect, 10), 0);
}

test(ProtocolTest, SendByteLiteral) {
    FakeWriteStream stream;
    Protocol p(&stream);

    uint8_t expect[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x64, 0x70, 0xFF, 0xFF, 0xFF};
    uint8_t actual[32];
    stream.set(actual, 32);

    p.send({0x70, 0x61, 0x67, 0x65, 0x20, 0x64, 0x70});
    assertEqual(memcmp(actual, expect, 10), 0);
}

test(ProtocolTest, SendStringLiteral) {
    FakeWriteStream stream;
    Protocol p(&stream);

    uint8_t expect[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x64, 0x70, 0xFF, 0xFF, 0xFF};
    uint8_t actual[32];
    stream.set(actual, 32);

    p.send("page dp");
    assertEqual(memcmp(actual, expect, 10), 0);
}

test(ProtocolTest, SendString) {
    FakeWriteStream stream;
    Protocol p(&stream);

    uint8_t expect[] = {0x70, 0x61, 0x67, 0x65, 0x20, 0x64, 0x70, 0xFF, 0xFF, 0xFF};
    uint8_t actual[32];
    stream.set(actual, 32);

    char cmd[] = "page dp";
    p.send(cmd);
    assertEqual(memcmp(actual, expect, 10), 0);
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
    TestRunner::run();
    delay(1);
}
