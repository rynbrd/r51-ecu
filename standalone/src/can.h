#ifndef __R51_CAN__
#define __R51_CAN__

#include <Canny.h>

#include "bus.h"


class CanNode : public Node {
    public:
        CanNode(Canny::Controller* can) :
            can_(can), retries_(5), frame_(8) {}

        // Write a frame to the CAN bus. Only frames which match filter are
        // written.
        virtual void handle(const Canny::Frame& frame) override;

        // Read a single CAN frame and broadcast it to the event bus.
        virtual void emit(const Yield& yield) override;

        // Only read CAN frames which match this filter.
        virtual bool readFilter(const Canny::Frame& frame) const = 0;

        // Only write CAN frames which match this filter.
        virtual bool writeFilter(const Canny::Frame& frame) const = 0;

    private:
        Canny::Controller* can_;
        uint8_t retries_;
        Canny::Frame frame_;
};

#endif  // __R51_CAN__
