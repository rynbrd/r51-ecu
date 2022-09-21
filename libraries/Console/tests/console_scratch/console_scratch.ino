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

test(ConsoleScratchTest, ReadEmpty) {
    Scratch scratch;
    strcpy((char*)buffer, "scratch\n");

    FakeReadStream stream;
    stream.set(buffer, strlen((char*)buffer));
    FakeYield yield;
    ConsoleNode console(&stream);

    console.emit(yield);
    assertEqual(scratch.size, (size_t)0);
    assertEqual(strcmp("", (char*)scratch.bytes), 0);
}

test(ConsoleScratchTest, ReadWord) {
    Scratch scratch;
    strcpy((char*)buffer, "scratch hello\n");

    FakeReadStream stream;
    stream.set(buffer, strlen((char*)buffer));
    FakeYield yield;
    ConsoleNode console(&stream, &scratch);

    console.emit(yield);
    assertEqual(scratch.size, (size_t)6);
    assertEqual(strcmp("hello", (char*)scratch.bytes), 0);
}

test(ConsoleScratchTest, ReadLine) {
    Scratch scratch;
    strcpy((char*)buffer, "scratch hello, world!!\n");

    FakeReadStream stream;
    stream.set(buffer, strlen((char*)buffer));
    FakeYield yield;
    ConsoleNode console(&stream, &scratch);

    console.emit(yield);
    assertEqual(scratch.size, (size_t)15);
    assertEqual(strcmp("hello, world!!", (char*)scratch.bytes), 0);
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
