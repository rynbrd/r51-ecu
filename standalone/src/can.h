#ifndef __R51_CAN__
#define __R51_CAN__

#include <Canny.h>
#include <Caster.h>

#include "events.h"


class CanNode : public Caster::Node<Message> {
    public:
        CanNode(Canny::Controller* can) :
            can_(can), retries_(5), frame_(8) {}

        // Write a frame to the CAN bus. Only frames which match filter are
        // written.
        virtual void handle(const Message& msg) override;

        // Read a single CAN frame and broadcast it to the event bus.
        virtual void emit(const Caster::Yield<Message>& yield) override;

        // Only read CAN frames which match this filter.
        virtual bool readFilter(const Message& msg) const = 0;

        // Only write CAN frames which match this filter.
        virtual bool writeFilter(const Message& msg) const = 0;

    private:
        Canny::Controller* can_;
        uint8_t retries_;
        Canny::Frame frame_;
};

#endif  // __R51_CAN__
