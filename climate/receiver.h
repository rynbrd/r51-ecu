#ifndef __R51_RECEIVER__
#define __R51_RECEIVER__

class Receiver {
    public:
        Receiver() {}
        virtual ~Receiver() {}

        // Read a frame from the receiver. Return true if a frame was read of
        // false if not.
        virtual bool read(uint32_t* id, uint8_t* len, byte* data) = 0;

        // Write frame to the receiver. Return true if the frame was written or
        // false if it was not.
        virtual bool write(uint32_t id, uint8_t len, byte* data) = 0;
};

#endif  // __R51_RECEIVER__
