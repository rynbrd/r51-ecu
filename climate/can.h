#ifndef __R51_CAN__
#define __R51_CAN__

#include "mcp_can.h"
#include "receiver.h"

class CanReceiver : public Receiver {
    public:
        CanReceiver() : mcp_(nullptr) {}
        ~CanReceiver();

        // Initialize the CAN controller. 
        bool begin(uint8_t cs_pin = 17, uint8_t baudrate = CAN_500KBPS, uint8_t clockset = MCP_16MHz);

        // Read a frame from the CAN bus. Return true if a frame was read.
        bool read(uint32_t* id, uint8_t* len, byte* data) override;

        // Write frame to the CAN bus. Return true if the frame was written.
        bool write(uint32_t id, uint8_t len, byte* data) override;
    private:
        MCP_CAN* mcp_;
};

#endif  // __R51_CAN__
