#ifndef _R51_PICO_PIPE_H_
#define _R51_PICO_PIPE_H_

#include <Arduino.h>
#include <Caster.h>
#include <Core.h>

extern "C" {
    #include <pico/util/queue.h>
};

namespace R51 {

class Pipe;

// Caster node implementation for Pipe. Do not use this directly.
class PipeNode : public Caster::Node<Message> {
    public:
        PipeNode(Pipe* parent, int8_t side) : parent_(parent), side_(side) {}

        // Send event to the write queue.
        void handle(const Message& msg, const Caster::Yield<Message>&) override;

        // Emit the next message in the read queue.
        void emit(const Caster::Yield<Message>& yield) override;
    private:
        Pipe* parent_;
        int8_t side_;

        queue_t* read_queue() const;
        queue_t* write_queue() const;
        bool filter(const Message& msg);
};

// Pipe allows two RP2040 cores to each run their own bus and communicate
// over it via a shared queue. A Pipe object exposes two nodes that should
// be added to the buses running on their respective cores.
//
// Messages received when a queue is full are discarded. The queue sizes should
// be carefully chosen to handle bursty writes to the bus. Filtering should be
// used to mitigate this, only transmitting relevant events across the cores.
class Pipe {
    public:
        // Construct a Pipe node with the given queue capacities. The left
        // and right nodes write to the left and right queues respectively.
        Pipe(size_t left_capacity, size_t right_capacity);

        // Destroy the object. Frees memory associated with the underlying queues.
        ~Pipe();

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

        PipeNode left_node_;
        PipeNode right_node_;

        friend class PipeNode;
};

}  // namespace R51

#endif  // _R51_PICO_PIPE_H_
