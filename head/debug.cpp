#include "debug.h"

#include <Arduino.h>
#include <stdint.h>

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

size_t Debug_::print(uint32_t id, uint8_t len, uint8_t* data) {
    size_t n = 0;
    n += print(id, HEX);
    n += print("#");
    for (int i = 0; i < len; i++) {
        if (data[i] <= 0x0F) {
            n += print("0");
        }
        n += print(data[i], HEX);
        if (i < len-1) {
            n += print(":");
        }
    }
    return n;
}

size_t Debug_::println(uint32_t id, uint8_t len, uint8_t* data) {
    print(id, len, data);
    print("\n");
}

Debug_ Debug;
