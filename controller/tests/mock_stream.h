#ifndef __R51_TESTS_MOCK_STREAM__
#define __R51_TESTS_MOCK_STREAM__

#include <Arduino.h>


// A fake stream for reading from a buffer.
class FakeReadStream : public Stream {
    public:
        FakeReadStream() : buffer_(nullptr), size_(0), pos_(0) {}

        // Return true if the buffer has not been exhausted.
        int available() override;

        // Read a single byte from the buffer and advance the position. Return
        // 0 if the buffer is exhausted.
        int read() override;

        // Return the byte at the current position or 0 if none is available.
        int peek() override;

        // Set the next buffer to read. This resets the current buffer. Does
        // not take ownership of the pointer.
        void set(byte* buffer, int len);

        // The number of bytes remaining in the read buffer.
        size_t remaining();

        // We don't use these so they are noops.
        size_t write(uint8_t) override { return 0; }
        size_t write(const uint8_t*, size_t) override { return 0; }

    private:
        byte* buffer_;
        int size_;
        int pos_;
};

// A fake stream for writing to a buffer.
class FakeWriteStream : public Stream {
    public:
        FakeWriteStream() : buffer_(nullptr), size_(0), pos_(0) {}

        // Write a byte to the buffer and advance the position. Returns 0 if
        // there is no more space in the buffer.
        size_t write(uint8_t byte) override;

        // Write up to len bytes to the buffer and advance the position.
        // Returns the remaining capacity if len is larger.
        size_t write(const uint8_t* data, size_t len) override;

        // Set the buffer to write to. This resets the current buffer. Does not
        // take ownership of the pointer.
        void set(byte* buffer, int len);

        // The number of bytes remaining in the read buffer.
        size_t remaining();

        // We don't use these so they are noops.
        int available() override { return 0; }
        int read() override { return 0; }
        int peek() override { return 0; }

    private:
        byte* buffer_;
        int size_;
        int pos_;
};

#endif  // __R51_TESTS_MOCK_STREAM__
