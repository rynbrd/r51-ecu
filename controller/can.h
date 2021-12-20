#ifndef __R51_CAN__
#define __R51_CAN__

#include "connection.h"
#include "same51_can.h"

class CanConnection : public Connection {
    public:
        CanConnection(uint32_t baudrate = CAN_500KBPS) :
            client_(), init_(false),
            baudrate_(baudrate), retries_(5) {}

        // Initialize the CAN controller. 
        bool begin();

        // Read a frame from the CAN bus. Return true if a frame was read.
        bool read(uint32_t* id, uint8_t* len, byte* data) override;

        // Write frame to the CAN bus. Return true if the frame was written.
        bool write(uint32_t id, uint8_t len, byte* data) override;
    private:
        SAME51_CAN client_;
        bool init_;
        uint32_t baudrate_;
        uint8_t retries_;
};

#endif  // __R51_CAN__
