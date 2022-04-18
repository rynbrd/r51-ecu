#ifndef __R51_SERIAL_H__
#define __R51_SERIAL_H__

#include <Arduino.h>
#include <Canny.h>

#include "bus.h"


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
class SerialText : public Node {
    public:
        SerialText() : stream_(nullptr), frame_(8) {}

        // Start receiving frames from the given stream. Typically Serial,
        // SerialUSB, or Serial1.
        void begin(Stream* stream);

        // Write a text frame to serial.
        void handle(const Canny::Frame& frame) override;

        // Read a text frame from serial and emit it to the bus.
        void emit(const Yield& yield) override;

        // Filter frames to receive from the serial connection. Defaults to
        // allowing all frames.
        virtual bool filter(const Canny::Frame& frame) const = 0;

    private:
        Stream* stream_;
        byte conv_[5];
        byte buffer_[32];
        uint8_t buffer_len_;
        uint8_t id_len_;
        uint8_t data_len_;
        Canny::Frame frame_;

        void reset();
};

#endif  // __R51_SERIAL_H__
