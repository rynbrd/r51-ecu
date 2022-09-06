#ifndef _R51_COMMON_CAN_
#define _R51_COMMON_CAN_

#include <Canny.h>
#include <Caster.h>

#include "Message.h"

namespace R51 {

// Bus node for reading and writing CAN frames to a CAN controller. Supports
// buffering of reads and writes to avoid frame loss on busy networks.
class CANNode : public Caster::Node<Message> {
    public:
        // Construct a new note that transmits frames over the given
        // connection.
        //
        // Read buffering is enabled when read_buffer_size > 1. On
        // emit the buffer is filled while frames are available to read without
        // waiting. One frame is emitted per emit call.
        //
        // Write buffering is enabled when write_buffer_size > 0. The handle
        // method will queue a CAN frame to the buffer when the controller
        // returns an ERR_FIFO on write. An attempt write the contents of the
        // buffer is made on each subsequent call to handle and on each call to
        // emit.
        CANNode(Canny::Connection* can, size_t read_buffer_size = 1, size_t write_buffer_size = 0);
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

        Canny::Frame* write_buffer_;
        size_t write_buffer_size_;
        size_t write_buffer_len_;
        size_t write_buffer_0_;

        void fillReadBuffer();
        Canny::Error drainWriteBuffer();
};

}  // namespace R51

#endif  // _R51_COMMON_CAN_
