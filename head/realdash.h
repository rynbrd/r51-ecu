#ifndef __REALDASH_H__
#define __REALDASH_H__

#include <Arduino.h>

// Reads and writes frames to RealDash over serial. Only supports RealDash 0x44
// type frames. Other incoming frame types are discarded.
class RealDash {
    public:
        // Construct an uninitialized RealDash instance.
        RealDash() : stream_(nullptr) {}

        // Start the RealDash instance. Data is transmitted over the given
        // serial stream. This is typically Serial or SerialUSB.
        void begin(Stream* stream);

        // Read a frame from RealDash. Returns true if a frame was read or
        // false if not. Should be called on every loop or the connected serial
        // device may block.
        bool read(uint32_t* id, uint8_t* len, byte* data);

        // Write frame to RealDash.
        void write(uint32_t id, uint8_t len, byte* data);

    private:
        Stream* stream_;

        // Incoming frame buffer.
        byte incoming_buffer_[17];

        // How many bytes have been written into the incoming buffer. This will
        // be 17 when the buffer is full and a frame is available for read.
        int8_t incoming_size_;
};

#endif
