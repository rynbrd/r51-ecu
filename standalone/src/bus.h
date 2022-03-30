#ifndef __R51_BUS__
#define __R51_BUS__

#include <Arduino.h>


// A data frame.
struct Frame {
    uint32_t id;
    uint8_t len;
    byte data[64];
};

// Check if two frames are equal.
bool frameEquals(const Frame& left, const Frame& right);

// Reset a frame. Set the frame's ID and length and zero out the data bytes.
void initFrame(Frame* frame, uint32_t id, uint8_t len);

// Copy contents of frame src to dest.
void copyFrame(Frame* dest, const Frame& src);

// Callable used by nodes to broadcast frames to the bus.
class Broadcast {
    public:
        // Broadcast a frame.
        virtual void operator()(const Frame& frame) const;
};

// A bus node. The bus receives frames from nodes. Frames received from nodes
// are broadcast to all nodes on the bus.
class Node {
    public:
        Node() = default;
        virtual ~Node() = default;

        // Receive frames from a node. The node calls broadcast() to put frames
        // on the bus. Frames are sent to all nodes whose filter matches the
        // broadcast frame.
        virtual void receive(const Broadcast& broadcast) = 0;

        // Send a frame to the node. Every received frame is sent to every node
        // on the bus whose filter method returns true. A node is not required
        // to process a frame.
        virtual void send(const Frame& frame) = 0;

        // Filter sent frames to this node. Return true for frame IDs that
        // should be sent to this node.
        virtual bool filter(uint32_t id) const = 0;
};

// Bus connects a series of nodes which send and receive frames. Frames are
// received sequentially from the connected nodes. Any time a frame is received
// it is broadcast to all connected nodes, including the originator of the
// frame.
class Bus {
    public:
        // Construct a bus that connects the provided set of nodes. Count is
        // the number of nodes in the array.
        Bus(Node** nodes, uint8_t count) : nodes_(nodes), count_(count), broadcast_(this) {}

        // Called on each main loop iteration. Calls receive on each node and
        // broadcasts any received frames.
        void loop();

    private:
        class BroadcastImpl : public Broadcast{
            public:
                void operator()(const Frame& frame) const override;

            private:
                Bus* bus_;
                BroadcastImpl(Bus* bus) : bus_(bus) {}
                friend class Bus;
        };


        Node** nodes_;
        uint8_t count_;
        Frame frame_;
        BroadcastImpl broadcast_;
};

#endif  // __R51_BUS__
