#ifndef _R51_COMMON_CAN_
#define _R51_COMMON_CAN_

#include <Canny.h>
#include <Caster.h>

#include "Message.h"

namespace R51 {

class CANNode : public Caster::Node<Message> {
    public:
        CANNode(Canny::Connection* can, size_t read_buffer_size = 1);
        virtual ~CANNode();

        // Write a frame to the CAN bus. Only frames which match filter are
        // written.
        void handle(const Message& msg) override;

        // Read a single CAN frame and broadcast it to the event bus.
        void emit(const Caster::Yield<Message>& yield) override;

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

        Canny::Frame* read_buffer_;
        size_t read_buffer_size_;
        size_t read_buffer_len_;
        size_t read_buffer_0_;

        void fillReadBuffer();
};

}  // namespace R51

#endif  // _R51_COMMON_CAN_
