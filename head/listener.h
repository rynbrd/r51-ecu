#ifndef __R51_LISTENER_H__
#define __R51_LISTENER_H__

#include <Arduino.h>

// Translates received frames and performs controller actions. Implementations
// should contain a "connect" method which connects the listener to the
// appropriate controllers.
class FrameListener {
    public:
        FrameListener() {}
        virtual ~FrameListener() {}

        // Receive and process a frame.
        void receive(uint32_t id, uint8_t len, byte* data);
};

#endif  // __R51_LISTENER_H__
