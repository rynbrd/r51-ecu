#ifndef __R51_CONNECTION__
#define __R51_CONNECTION__

#include <Arduino.h>

class Connection {
    public:
        Connection() {}
        virtual ~Connection() {}

        // Read a frame from the connection. Return true if a frame was read of
        // false if not.
        virtual bool read(uint32_t* id, uint8_t* len, byte* data) = 0;

        // Write a frame to the connection. Return true if the frame was
        // written or false if it was not.
        virtual bool write(uint32_t id, uint8_t len, byte* data) = 0;
};

#endif  // __R51_CONNECTION__
