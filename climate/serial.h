#ifndef __R51_SERIAL_H__
#define __R51_SERIAL_H__

#include <Arduino.h>
#include "receiver.h"

// A text based receiver which reads frames from serial. Useful for sending raw
// frames for debugging via ttl.
//
// Frames are in the form:
//   HHHHHHHH#00:00:00:00:00:00:00:00
//
// Or:
//   HHHHHHHH#0000000000000000
//
// HHHHHHHH is the 32-bit hex ID of the frame. It may be from 0 to 8
// characters. Each 00 is a byte in the data payload. Fewer bytes may be
// provided for shorter payloads. Each frame is terminated in a newline.
class SerialReceiver : public Receiver {
    public:
        SerialReceiver() : stream_(nullptr) {}

        // Start receiving frames from the given stream. Typically Serial,
        // SerialUSB, or Serial1.
        void begin(Stream* stream);

        // Read a frame from serial. Returns false if no frame was read.
        bool read(uint32_t* id, uint8_t* len, byte* data) override;

        // Print the frame to serial.
        bool write(uint32_t id, uint8_t len, byte* data) override;

    private:
        Stream* stream_;
        byte conv_[5];
        byte buffer_[32];
        uint8_t buffer_len_;
        uint8_t id_len_;
        uint8_t data_len_;

        void reset();
};

#endif  // __R51_SERIAL_H__
