#ifndef __R51_SERIAL_H__
#define __R51_SERIAL_H__

#include <Arduino.h>

// A text based receiver which reads frames from serial. Useful for sending raw
// frames for debugging via ttl.
//
// Frames are in the form:
//   HHHH#00:00:00:00:00:00:00:00
//
// Or:
//   HHHH#0000000000000000
//
// HHHH is the hex ID for standard frames and 00 is each byte in the data
// payload. Fewer bytes may be provided for shorter payloads. Each frame is
// terminated in a newline.
class SerialReceiver {
    public:
        SerialReceiver() : stream_(nullptr) {}

        // Start receiving frames from the given stream. Typically Serial,
        // SerialUSB, or Serial1.
        void begin(Stream* stream);

        // Read a frame from serial. Returns false if no frame was read.
        bool read(uint32_t* id, uint8_t* len, byte* data);

    private:
        Stream* stream_;
        byte conv_[5];
        byte buffer_[28];
        uint8_t buffer_len_;
        uint8_t id_len_;
        uint8_t data_len_;

        void reset();
};

#endif  // __R51_SERIAL_H__
