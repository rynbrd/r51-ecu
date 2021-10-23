#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <Stream.h>
#include <stdint.h>

// Stream wrapper class used for debug logging. Reading is not supported.
class Debug_ : public Stream {
    public:
        // Create an uninitialized debug logger. Methods other than begin() are
        // a noop. Calling begin() will activate the debug logger.
        Debug_() : stream_(nullptr) {}

        // Start debug logging to the given stream. This activates the other
        // methods and their calls are forwarded to stream.
        void begin(Stream* stream);

        // Stop debug logging to the stream. This deactivates methods other
        // than begin() until begin() is called again.
        void stop();

        // Write a character.
        size_t write(uint8_t ch) override;

        // Write a string.
        size_t write(const uint8_t *str, size_t size) override;

        // Check if data is available to read.
        int available() override;

        // Read a character.
        int read() override;

        // Peek.
        int peek() override;

        // Print a human readable frame.
        size_t print(uint32_t id, uint8_t len, uint8_t* date);
        size_t println(uint32_t id, uint8_t len, uint8_t* date);

        using Stream::print;
        using Stream::println;
    private:
        Stream* stream_;
};

extern Debug_ Debug;

#endif
