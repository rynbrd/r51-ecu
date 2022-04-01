#ifndef __R51_CAN__
#define __R51_CAN__

#include "bus.h"
#include "CANBed.h"


class CanNode : public Node {
    public:
        CanNode(CANBed::Bitrate bitrate = CANBed::CAN20_500K) :
            init_(false), bitrate_(bitrate), retries_(5) {}

        // Initialize the CAN controller. 
        void begin();

        // Receive a frame from the CAN bus.
        virtual void receive(const Broadcast& broadcast) override;

        // Send a frame to the CAN bus.
        virtual void send(const Frame& frame) override;
    private:
        bool init_;
        CANBed::Bitrate bitrate_;
        uint8_t retries_;
        Frame frame_;
};

#endif  // __R51_CAN__
