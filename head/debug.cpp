#include "debug.h"

#include <Arduino.h>
#include <stdint.h>
#include "can.h"
#include "status.h"

namespace ECU {

void Debug_::begin(Stream* stream) {
  stream_ = stream;
}

void Debug_::stop() {
  stream_ = nullptr;
}

size_t Debug_::write(uint8_t ch) {
  if (stream_ == nullptr) {
    return 1;
  }
  return stream_->write(ch);
}

size_t Debug_::write(const uint8_t *str, size_t size) {
  if (stream_ == nullptr) {
    return size;
  }
  return stream_->write(str, size);
}

int Debug_::available() {
  if (stream_ == nullptr) {
    return 0;
  }
  return stream_->available();
}

int Debug_::read() {
  if (stream_ == nullptr) {
    return -1;
  }
  return stream_->read();
}

int Debug_::peek() {
  if (stream_ == nullptr) {
    return -1;
  }
  return stream_->peek();
}

size_t Debug_::print(const Frame& frame) {
    if (stream_ == nullptr) {
        return 0;
    }
    return printFrame(stream_, frame);
}

// Print a frame and a newline.
size_t Debug_::println(const Frame& frame) {
    if (stream_ == nullptr) {
        return 0;
    }
    return print(frame) + print("\n");
}

size_t Debug_::print(Status status) {
    if (stream_ == nullptr) {
        return 0;
    }
    return printStatus(stream_, status);
}

size_t Debug_::println(Status status) {
    if (stream_ == nullptr) {
        return 0;
    }
    return print(status) + print("\n");
}

Debug_ Debug;

}
