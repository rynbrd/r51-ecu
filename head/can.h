#ifndef __ECU_CAN_H__
#define __ECU_CAN_H__

#include <stdint.h>
#include <Stream.h>
#include "status.h"

namespace ECU {

typedef enum : int8_t {
    FRAME_STD = 0,    // CAN 2.0A/2.0B standard frame. Contains an 11 bit ID.
    FRAME_EXT = 1,    // Can 2.0B extended frame. Contains a 29 bit ID.
} FrameType;

// Raw frame storage.
struct Frame {
    FrameType type = FRAME_STD;
    uint8_t size;
    uint32_t id;
    uint8_t data[8];
};

// Print a human readable frame to a stream.
size_t printFrame(Stream* stream, const Frame& frame);

typedef enum : int8_t {
    CAN_SPEED_250K,
    CAN_SPEED_500K,
} CanSpeed;

// Base class for all CAN tranceivers.
class CanXcvr {
    public:
        CanXvcr() {}
        virtual ~CanXcvr() {}

        // Start the tranceiver. Must be called before any other method. Return
        // OK on success or ERROR on failure. 
        virtual Status begin();

        // Return true if a message is available to read.
        virtual bool available() const;

        // Recieve a frame. Return OK on success, ERROR on failure, or NOENT if
        // there is no frame ready to be read.
        virtual Status receive(Frame* frame) const;

        // Send a frame. Return OK on success or ERROR on failure.
        virtual Status send(const Frame& frame);
};

}

#endif
