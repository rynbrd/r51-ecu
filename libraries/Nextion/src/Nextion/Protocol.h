#ifndef _NEXTION_PROTOCOL_H_
#define _NEXTION_PROTOCOL_H_

#include <Arduino.h>

namespace Nextion {

// Implements low level operations to interact with Nextion HMI displays.
class Protocol {
    public:
        // Constructa new Nextion protocol object that communicates over the
        // given stream.
        Protocol(Stream* stream) : stream_(stream) {}

        // Send a raw command to the display. The command should not include
        // the terminating 0XFFFFFF bytes.
        void send(const uint8_t* command, size_t size);
        void send(const char* command);

        template <size_t N> 
        void send(const uint8_t (&command)[N]);

        // Receive a raw message from the display. The message is written into
        // message byte array and the size of the message is returned. The
        // terminating 0xFFFFFF bytes are not included. Returns the size of the
        // message read from the stream not including the terminating
        // characters.
        //
        // If bytes are available to read from the stream then this method will
        // block until the terminating bytes are received from the display. The
        // Nextion HMI devices read and write the entire serial buffer while
        // blocking display activity, so there should be no case where this
        // function needs to wait for the display to perform background operations.
        //
        // The buffer size is provided by size must be large enough to hold the
        // message and its terminating characters. If a buffer overrun occurs
        // then 0 is returned and the message is discarded.
        size_t recv(uint8_t* message, size_t size);

    private:
        Stream* stream_;
};

template <size_t N> 
void Protocol::send(const uint8_t (&command)[N]) {
    send((uint8_t*)command, N);
}

}  // namespace Nextion

#endif  // _NEXTION_PROTOCOL_H_
