#ifndef _R51_CORE_CAN_H_
#define _R51_CORE_CAN_H_

#include <Canny.h>
#include <Caster.h>

#include "Message.h"

namespace R51 {

// Bus node for reading and writing frames to a CAN 2.0 controller.
class CANGateway : public Caster::Node<Message> {
    public:
        // Construct a new note that transmits frames over the given
        // connection.
        CANGateway(Canny::Connection<Canny::CAN20Frame>* can) : can_(can) {}
        virtual ~CANGateway() = default;

        // Write a frame to the CAN bus.
        void handle(const Message& msg, const Caster::Yield<Message>&) override;

        // Read a single CAN frame and broadcast it to the event bus.
        void emit(const Caster::Yield<Message>& yield) override;

        // Called when a frame can't be read from the bus.
        virtual void onReadError(Canny::Error) {}

        // Called when a frame can't be written to the bus.
        virtual void onWriteError(Canny::Error, const Canny::CAN20Frame&) {}

    private:
        Canny::Connection<Canny::CAN20Frame>* can_;
        Canny::CAN20Frame frame_;
};

}  // namespace R51

#endif  // _R51_CORE_CAN_H_
