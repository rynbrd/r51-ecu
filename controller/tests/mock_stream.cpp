#include "mock_stream.h"

int FakeReadStream::available() {
    return pos_ < size_;
}

int FakeReadStream::read() {
    if (!available()) {
        return 0;
    }
    return buffer_[pos_++];
}

int FakeReadStream::peek() {
    if (!available()) {
        return 0;
    }
    return buffer_[pos_];
}

void FakeReadStream::set(byte* buffer, int len) {
    buffer_ = buffer;
    size_ = len;
    pos_ = 0;
}

size_t FakeReadStream::remaining() {
    return size_ - pos_;
}

size_t FakeWriteStream::write(uint8_t byte) {
    if (remaining() == 0) {
        return 0;
    }
    buffer_[pos_++] = byte;
    return 1;
}

size_t FakeWriteStream::write(const uint8_t* data, size_t len) {
    if (remaining() < len) {
        len = remaining();
    }
    for (uint32_t i = 0; i < len; i++) {
        buffer_[pos_++] = data[i];
    }
    return len;
}

void FakeWriteStream::set(byte* buffer, int len) {
    buffer_ = buffer;
    size_ = len;
    pos_ = 0;
}

size_t FakeWriteStream::remaining() {
    return size_ - pos_;
}
