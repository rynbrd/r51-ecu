#ifndef _R51_COMMON_CAN_
#define _R51_COMMON_CAN_

#include <Canny.h>
#include <Caster.h>

#include "Message.h"

namespace R51 {

class CANNode : public Caster::Node<Message> {
    public:
        CANNode(Canny::Connection* can) :
            can_(can), retries_(5), frame_(8) {}

        // Write a frame to the CAN bus. Only frames which match filter are
        // written.
        virtual void handle(const Message& msg) override;

        // Read a single CAN frame and broadcast it to the event bus.
        virtual void emit(const Caster::Yield<Message>& yield) override;

        // Only read CAN frames which match this filter.
        virtual bool readFilter(const Canny::Frame&) const { return true; }

        // Only write CAN frames which match this filter.
        virtual bool writeFilter(const Canny::Frame&) const { return true; }

        // Called when a frame can't be read from the bus.
        virtual void onReadError(Canny::Error) {}

        // Called when a frame can't be written to the bus.
        virtual void onWriteError(Canny::Error, const Canny::Frame&) {}


    private:
        Canny::Connection* can_;
        uint8_t retries_;
        Canny::Frame frame_;
};

}  // namespace R51

#endif  // _R51_COMMON_CAN_
