#ifndef __R51_SERIAL_H__
#define __R51_SERIAL_H__

#include <Arduino.h>
#include "connection.h"


#define WAIT_FOR_SERIAL(SERIAL, DELAY, LOG_MSG) ({\
    while(!SERIAL) {\
        if (LOG_MSG != nullptr) {\
            INFO_MSG(LOG_MSG);\
        }\
        delay(DELAY);\
    }\
})


// A text based connection which reads frames from a serial stream. Useful for
// sending hand-built frames for debugging via TTL.
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
//
// Frames written to the stream will be printed in the above form and end in a
// CR+LF.
class SerialConnection : public Connection {
    public:
        SerialConnection() : stream_(nullptr) {}

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
