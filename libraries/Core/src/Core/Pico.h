#ifndef _R51_CORE_PICO_H_
#define _R51_CORE_PICO_H_

#include <Arduino.h>

#ifdef RASPBERRYPI_PICO

#include <Caster.h>
#include "Message.h"

extern "C" {
    #include <pico/util/queue.h>
};

namespace R51 {

class PicoPipe;

// Caster node implementation for PicoPipe. Do not use this directly.
class PicoPipeNode : public Caster::Node<Message> {
    public:
        PicoPipeNode(PicoPipe* parent, int8_t side) : parent_(parent), side_(side) {}

        // Send event to the write queue.
        void handle(const Message& msg, const Caster::Yield<Message>&) override;

        // Emit the next message in the read queue.
        void emit(const Caster::Yield<Message>& yield) override;
    private:
        PicoPipe* parent_;
        int8_t side_;

        queue_t* read_queue() const;
        queue_t* write_queue() const;
        bool filter(const Message& msg);
};

// PicoPipe allows two RP2040 cores to each run their own bus and communicate
// over it via a shared queue. A PicoPipe object exposes two nodes that should
// be added to the buses running on their respective cores.
//
// Messages received when a queue is full are discarded. The queue sizes should
// be carefully chosen to handle bursty writes to the bus. Filtering should be
// used to mitigate this, only transmitting relevant events across the cores.
class PicoPipe {
    public:
        // Construct a PicoPipe node with the given queue capacities. The left
        // and right nodes write to the left and right queues respectively.
        PicoPipe(size_t left_capacity, size_t right_capacity);

        // Destroy the object. Frees memory associated with the underlying queues.
        ~PicoPipe();

        // Return a pointer to the "left" node. Messages received by this node
        // are yielded to the "right" node's bus.
        Caster::Node<Message>* left() { return &left_node_; }

        // Filter events handled by the "left" node. Discard events for which
        // false is returned.
        virtual bool filterLeft(const Message&) { return true; }

        // Return a pointer to the "right" node. Messages received by this node
        // are yielded to the "left" node's bus.
        Caster::Node<Message>* right() { return &right_node_; }

        // Filter messages handled by the "right" node. Discard events for which false is returned.
        virtual bool filterRight(const Message&) { return true; }

        // Called when a message must be discarded due to insufficient capacity.
        virtual void onBufferOverrun(const Message&) {}

    private:
        queue_t left_queue_;    // Left node produces to this queue.
        queue_t right_queue_;   // Right node produces to this queue.

        PicoPipeNode left_node_;
        PicoPipeNode right_node_;

        friend class PicoPipeNode;
};

}  // namespace R51

#endif  // RASPBERRYPI_PICO
#endif  // _R51_CORE_PICO_H_
