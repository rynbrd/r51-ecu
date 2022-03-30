#ifndef __R51_CAN__
#define __R51_CAN__

#include "bus.h"
#include "same51_can.h"


class Same51Can : public Node {
    public:
        Same51Can(uint32_t baudrate = CAN_500KBPS) :
            client_(), init_(false),
            baudrate_(baudrate), retries_(5) {}

        // Initialize the CAN controller. 
        void begin();

        // Receive a frame from the CAN bus.
        virtual void receive(const Broadcast& broadcast) override;

        // Send a frame to the CAN bus.
        virtual void send(const Frame& frame) override;
    private:
        SAME51_CAN client_;
        bool init_;
        uint32_t baudrate_;
        uint8_t retries_;
        Frame frame_;
};

#endif  // __R51_CAN__
