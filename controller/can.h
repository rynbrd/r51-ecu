#ifndef __R51_CAN__
#define __R51_CAN__

#include "connection.h"
#include "same51_can.h"

class CanConnection : public Connection {
    public:
        enum MaskId : uint8_t {
            RXM0 = 0,   // RXB0
            RXM1 = 1,   // RXB1
        };

        enum FilterId : uint8_t {
            RXF0 = 0,   // RXB0
            RXF1 = 1,   // RXB0
            RXF2 = 2,   // RXB1
            RXF3 = 3,   // RXB1
            RXF4 = 4,   // RXB1
            RXF5 = 5,   // RXB1
        };

        CanConnection(uint32_t baudrate = CAN_500KBPS) :
            client_(), init_(false),
            baudrate_(baudrate), retries_(5) {}

        // Set a mask on the MCU. Return false on failure.
        bool setMask(MaskId id, uint16_t data);

        // Set a filter on the MCU. Return false on failure.
        bool setFilter(FilterId id, uint16_t data);

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
