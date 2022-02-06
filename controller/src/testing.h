#ifndef __R51_TESTING__ 
#define __R51_TESTING__ 

#include "connection.h"

// A connection that discards writes and never reads.
class NullConnection : public Connection {
    public:
        // Construct a new null connection.
        NullConnection() {}

        // Reads nothing. Always returns false.
        bool read(uint32_t*, uint8_t*, byte*) override {
            return false;
        };

        // Pretend to write a frame. Always returns true.
        bool write(uint32_t, uint8_t, byte*) override {
            return true;
        };
};

#endif  // __R51_TESTING__ 
