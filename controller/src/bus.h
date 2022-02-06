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
bool framesEqual(Frame* left, Frame* right);

// A bus node. The bus receives frames from nodes. Frames received from nodes
// are broadcast to all nodes on the bus.
class Node {
    public:
        Node() = default;
        virtual ~Node() = default;

        // Receive a frame from the node. Return true if a frame was read or false
        // otherwise. A frame received in this way is broadcast to all bus
        // nodes. The bus maintains ownership of the pointer.
        virtual bool receive(Frame* frame) = 0;

        // Send a frame to the node. Every received frame is sent to every node
        // on the bus. A node is not required to process a frame. The bus
        // maintains ownership of the pointer.
        virtual void send(Frame* frame) = 0;

        // Filter sent frames to this node. Return true for frame IDs that
        // should be sent to this node.
        virtual bool filter(uint32_t id) = 0;
};

// Bus connects a series of nodes which send and receive frames. Frames are
// received sequentially from the connected nodes. Any time a frame is received
// it is broadcast to all connected nodes, including the originator of the
// frame.
class Bus {
    public:
        Bus(Node** nodes, uint8_t count) : nodes_(nodes), count_(count) {}

        // Called on each main loop iteration. Calls receive on each node and
        // broadcasts any received frames.
        void loop();

    private:
        Node** nodes_;
        uint8_t count_;
        Frame frame_;

        // Broadcast the buffered frame.
        void broadcast();
};

#endif  // __R51_BUS__
