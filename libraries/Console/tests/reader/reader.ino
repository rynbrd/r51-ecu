#include <AUnit.h>
#include <Arduino.h>
#include <Common.h>
#include <Console/Reader.h>
#include <Faker.h>
#include <Test.h>

namespace R51 {

using namespace aunit;
using ::Faker::FakeReadStream;

char stream_buffer[64];
char reader_buffer[64];
const size_t buffer_size = 64;

test(ReaderTest, ReadWordEmpty) {
    strcpy(stream_buffer, "");
    FakeReadStream stream;
    stream.set((uint8_t*)stream_buffer, strlen(stream_buffer));

    size_t len = 0;
    Reader reader(&stream, reader_buffer, buffer_size);
    
    Reader::Error err = reader.word(&len);
    assertEqual(err, Reader::FIFO);
    assertEqual((uint16_t)len, (uint16_t)0);
}

test(ReaderTest, ReadWordSingleWithNewline) {
    strcpy(stream_buffer, "hello\n");
    FakeReadStream stream;
    stream.set((uint8_t*)stream_buffer, strlen(stream_buffer));

    size_t len = 0;
    Reader reader(&stream, reader_buffer, buffer_size);
    
    Reader::Error err = reader.word(&len);
    assertEqual(err, Reader::EOL);
    assertStringsEqual(reader_buffer, "hello");
    assertEqual((uint16_t)len, 5);

    err = reader.word();
    assertEqual(err, Reader::FIFO);
}

test(ReaderTest, ReadWordSingleWithCarriageReturn) {
    strcpy(stream_buffer, "hello\r\n");
    FakeReadStream stream;
    stream.set((uint8_t*)stream_buffer, strlen(stream_buffer));

    size_t len = 0;
    Reader reader(&stream, reader_buffer, buffer_size);
    
    Reader::Error err = reader.word(&len);
    assertEqual(err, Reader::EOL);
    assertStringsEqual(reader_buffer, "hello");
    assertEqual((uint16_t)len, 5);

    err = reader.word();
    assertEqual(err, Reader::FIFO);
}

test(ReaderTest, ReadWordMultipleWords) {
    strcpy(stream_buffer, "hello world\n");
    FakeReadStream stream;
    stream.set((uint8_t*)stream_buffer, strlen(stream_buffer));

    size_t len = 0;
    Reader reader(&stream, reader_buffer, buffer_size);
    
    Reader::Error err = reader.word(&len);
    assertEqual(err, Reader::EOW);
    assertStringsEqual(reader_buffer, "hello");
    assertEqual((uint16_t)len, 5);

    err = reader.word();
    assertEqual(err, Reader::EOL);
    assertStringsEqual(reader_buffer, "world");

    err = reader.word();
    assertEqual(err, Reader::FIFO);
}

test(ReaderTest, ReadWordMultipleLines) {
    strcpy(stream_buffer, "hello world\r\ngoodbye world\r\n");
    FakeReadStream stream;
    stream.set((uint8_t*)stream_buffer, strlen(stream_buffer));

    Reader reader(&stream, reader_buffer, buffer_size);
    
    Reader::Error err = reader.word();
    assertEqual(err, Reader::EOW);
    assertStringsEqual(reader_buffer, "hello");

    err = reader.word();
    assertEqual(err, Reader::EOL);
    assertStringsEqual(reader_buffer, "world");

    err = reader.word();
    assertEqual(err, Reader::EOW);
    assertStringsEqual(reader_buffer, "goodbye");

    err = reader.word();
    assertEqual(err, Reader::EOL);
    assertStringsEqual(reader_buffer, "world");

    err = reader.word();
    assertEqual(err, Reader::FIFO);
}

test(ReaderTest, ReadWordLeadingWhitespace) {
    strcpy(stream_buffer, "  hello\r\n");
    FakeReadStream stream;
    stream.set((uint8_t*)stream_buffer, strlen(stream_buffer));

    size_t len = 0;
    Reader reader(&stream, reader_buffer, buffer_size);
    
    Reader::Error err = reader.word(&len);
    assertEqual(err, Reader::EOL);
    assertStringsEqual(reader_buffer, "hello");
    assertEqual((uint16_t)len, 5);

    err = reader.word();
    assertEqual(err, Reader::FIFO);
}

test(ReaderTest, ReadWordExtraWhitespace) {
    strcpy(stream_buffer, "hello    world\n");
    FakeReadStream stream;
    stream.set((uint8_t*)stream_buffer, strlen(stream_buffer));

    Reader reader(&stream, reader_buffer, buffer_size);
    
    Reader::Error err = reader.word();
    assertEqual(err, Reader::EOW);
    assertStringsEqual(reader_buffer, "hello");

    err = reader.word();
    assertEqual(err, Reader::EOL);
    assertStringsEqual(reader_buffer, "world");

    err = reader.word();
    assertEqual(err, Reader::FIFO);
}

test(ReaderTest, ReadWordTrailingWhitespace) {
    strcpy(stream_buffer, "hello    \r\n");
    FakeReadStream stream;
    stream.set((uint8_t*)stream_buffer, strlen(stream_buffer));

    Reader reader(&stream, reader_buffer, buffer_size);
    
    Reader::Error err = reader.word();
    assertEqual(err, Reader::EOL);
    assertStringsEqual(reader_buffer, "hello");

    err = reader.word();
    assertEqual(err, Reader::FIFO);
}

test(ReaderTest, ReadLineEmpty) {
    strcpy(stream_buffer, "");
    FakeReadStream stream;
    stream.set((uint8_t*)stream_buffer, strlen(stream_buffer));

    Reader reader(&stream, reader_buffer, buffer_size);
    
    Reader::Error err = reader.line();
    assertEqual(err, Reader::FIFO);
}

test(ReaderTest, ReadLineSingle) {
    strcpy(stream_buffer, "hello world\r\n");
    FakeReadStream stream;
    stream.set((uint8_t*)stream_buffer, strlen(stream_buffer));

    size_t len = 0;
    Reader reader(&stream, reader_buffer, buffer_size);
    
    Reader::Error err = reader.line(&len);
    assertEqual(err, Reader::EOL);
    assertStringsEqual(reader_buffer, "hello world");
    assertEqual((uint16_t)len, 11);

    err = reader.word();
    assertEqual(err, Reader::FIFO);
}

test(ReaderTest, ReadLineMultiple) {
    strcpy(stream_buffer, "hello world\r\ngoodbye world\r\n");
    FakeReadStream stream;
    stream.set((uint8_t*)stream_buffer, strlen(stream_buffer));

    Reader reader(&stream, reader_buffer, buffer_size);
    
    Reader::Error err = reader.line();
    assertEqual(err, Reader::EOL);
    assertStringsEqual(reader_buffer, "hello world");

    err = reader.line();
    assertEqual(err, Reader::EOL);
    assertStringsEqual(reader_buffer, "goodbye world");

    err = reader.word();
    assertEqual(err, Reader::FIFO);
}

test(ReaderTest, ReadLineLeadingWhitespace) {
    strcpy(stream_buffer, "   hello world\r\n");
    FakeReadStream stream;
    stream.set((uint8_t*)stream_buffer, strlen(stream_buffer));

    Reader reader(&stream, reader_buffer, buffer_size);
    
    Reader::Error err = reader.line();
    assertEqual(err, Reader::EOL);
    assertStringsEqual(reader_buffer, "hello world");

    err = reader.word();
    assertEqual(err, Reader::FIFO);
}

test(ReaderTest, ReadLineTrailingWhitespace) {
    strcpy(stream_buffer, "hello world    \r\n");
    FakeReadStream stream;
    stream.set((uint8_t*)stream_buffer, strlen(stream_buffer));

    Reader reader(&stream, reader_buffer, buffer_size);
    
    Reader::Error err = reader.line();
    assertEqual(err, Reader::EOL);
    assertStringsEqual(reader_buffer, "hello world");

    err = reader.word();
    assertEqual(err, Reader::FIFO);
}

test(ReaderTest, ReadWordThenLine) {
    strcpy(stream_buffer, "hello cruel world\r\n");
    FakeReadStream stream;
    stream.set((uint8_t*)stream_buffer, strlen(stream_buffer));

    Reader reader(&stream, reader_buffer, buffer_size);
    
    Reader::Error err = reader.word();
    assertEqual(err, Reader::EOW);
    assertStringsEqual(reader_buffer, "hello");

    err = reader.line();
    assertEqual(err, Reader::EOL);
    assertStringsEqual(reader_buffer, "cruel world");

    err = reader.word();
    assertEqual(err, Reader::FIFO);
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
