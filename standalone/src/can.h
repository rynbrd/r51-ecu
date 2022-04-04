#ifndef __R51_CAN__
#define __R51_CAN__

#include <Canny.h>

#include "bus.h"


class CanNode : public Node {
    public:
        CanNode(Canny::Controller* can) :
            can_(can), retries_(5), frame_(8) {}

        // Receive a frame from the CAN bus.
        virtual void receive(const Broadcast& broadcast) override;

        // Send a frame to the CAN bus.
        virtual void send(const Canny::Frame& frame) override;
    private:
        Canny::Controller* can_;
        uint8_t retries_;
        Canny::Frame frame_;
};

#endif  // __R51_CAN__
