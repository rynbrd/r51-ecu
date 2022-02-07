#ifndef __R51_TESTS_TEST_BUS__
#define __R51_TESTS_TEST_BUS__

#include "src/bus.h"


// A mock bus node for testing.
class MockNode : public Node {
    public:
        MockNode(int buffer_size) :
            send_(new Frame*[buffer_size]),
            send_size_(buffer_size),
            send_count_(0) {
            for (int i = 0; i < send_size_; i++) {
                send_[i] = nullptr;
            }
        }

        Frame* receive_ = nullptr;
        Frame* receive_extra_ = nullptr;
        Frame** send_;
        int send_size_;
        int send_count_;
        int64_t filter1_ = 0;
        uint32_t filter2_ = 0;

        ~MockNode() {
            if (receive_ != nullptr) {
                delete receive_;
            }
            if (receive_extra_ != nullptr) {
                delete receive_extra_;
            }
            for (int i = 0; i < send_size_; i++) {
                if (send_[i] != nullptr) {
                    delete send_[i];
                }
            }
            delete send_;
        }

        void receive(const Broadcast& broadcast) override {
            if (receive_ != nullptr) {
                broadcast(*receive_);
            }
            if (receive_extra_ != nullptr) {
                broadcast(*receive_extra_);
            }
        }

        void send(const Frame& frame) override {
            if (send_count_ < send_size_) {
                send_[send_count_] = new Frame();
                send_[send_count_]->id = frame.id;
                send_[send_count_]->len = frame.len;
                memcpy(send_[send_count_]->data, frame.data, frame.len);
            }
            ++send_count_;
        }

        bool filter(uint32_t id) override {
            return filter1_ == -1 || filter1_ == id || filter2_ == id;
        }
};

test(BusTest, SingleBroadcast) {
    MockNode n1 = MockNode(1);
    n1.receive_ = new Frame();
    n1.receive_->id = 1;
    n1.receive_->len = 2;
    n1.receive_->data[0] = 0x11;
    n1.receive_->data[1] = 0x22;

    MockNode n2 = MockNode(1);
    n2.receive_ = nullptr;
    n2.filter1_ = 1;

    MockNode n3 = MockNode(1);
    n3.receive_ = nullptr;
    n3.filter1_ = 1;

    Node* nodes[] = {&n1, &n2, &n3};

    Bus bus = Bus(nodes, sizeof(nodes)/sizeof(nodes[0]));
    bus.loop();

    assertEqual(n1.send_count_, 0);
    assertEqual(n2.send_count_, 1);
    assertTrue(frameEquals(*n1.receive_, *n2.send_[0]));
    assertEqual(n3.send_count_, 1);
    assertTrue(frameEquals(*n1.receive_, *n3.send_[0]));
}

test(BusTest, MultiBroadcast) {
    MockNode n1 = MockNode(1);
    n1.receive_ = new Frame();
    n1.receive_->id = 1;
    n1.receive_->len = 2;
    n1.receive_->data[0] = 0x11;
    n1.receive_->data[1] = 0x22;
    n1.filter1_ = 2;

    MockNode n2 = MockNode(1);
    n2.receive_ = new Frame();
    n2.receive_->id = 2;
    n2.receive_->len = 2;
    n2.receive_->data[0] = 0x33;
    n2.receive_->data[1] = 0x44;
    n2.filter1_ = 1;

    MockNode n3 = MockNode(2);
    n3.receive_ = nullptr;
    n3.filter1_ = 1;
    n3.filter2_ = 2;

    Node* nodes[] = {&n1, &n2, &n3};

    Bus bus = Bus(nodes, sizeof(nodes)/sizeof(nodes[0]));
    bus.loop();

    assertEqual(n1.send_count_, 1);
    assertTrue(frameEquals(*n2.receive_, *n1.send_[0]));
    assertEqual(n2.send_count_, 1);
    assertTrue(frameEquals(*n1.receive_, *n2.send_[0]));
    assertEqual(n3.send_count_, 2);
    assertTrue(frameEquals(*n1.receive_, *n3.send_[0]));
    assertTrue(frameEquals(*n2.receive_, *n3.send_[1]));
}

test(BusTest, MultiReceive) {
    MockNode n1 = MockNode(1);
    n1.receive_ = new Frame();
    n1.receive_->id = 1;
    n1.receive_->len = 2;
    n1.receive_->data[0] = 0x11;
    n1.receive_->data[1] = 0x22;
    n1.receive_extra_ = new Frame();
    n1.receive_extra_->id = 2;
    n1.receive_extra_->len = 2;
    n1.receive_extra_->data[0] = 0x33;
    n1.receive_extra_->data[1] = 0x44;

    MockNode n2 = MockNode(2);
    n2.receive_ = nullptr;
    n2.filter1_ = 1;
    n2.filter2_ = 2;

    Node* nodes[] = {&n1, &n2};

    Bus bus = Bus(nodes, sizeof(nodes)/sizeof(nodes[0]));
    bus.loop();

    assertEqual(n1.send_count_, 0);
    assertEqual(n2.send_count_, 2);
    assertTrue(frameEquals(*n1.receive_, *n2.send_[0]));
    assertTrue(frameEquals(*n1.receive_extra_, *n2.send_[1]));
}

test(BusTest, MultiLoop) {
    MockNode n1 = MockNode(1);
    n1.receive_ = new Frame();
    n1.receive_->id = 1;
    n1.receive_->len = 2;
    n1.receive_->data[0] = 0x11;
    n1.receive_->data[1] = 0x22;

    MockNode n2 = MockNode(1);
    n2.receive_ = nullptr;

    MockNode n3 = MockNode(3);
    n3.receive_ = nullptr;
    n3.filter1_ = 1;
    n3.filter2_ = 2;

    Node* nodes[] = {&n1, &n2, &n3};

    Bus bus = Bus(nodes, sizeof(nodes)/sizeof(nodes[0]));
    bus.loop();

    assertEqual(n1.send_count_, 0);
    assertEqual(n2.send_count_, 0);
    assertEqual(n3.send_count_, 1);
    assertTrue(frameEquals(*n1.receive_, *n3.send_[0]));

    n2.receive_ = new Frame();
    n2.receive_->id = 2;
    n2.receive_->len = 2;
    n2.receive_->data[0] = 0x33;
    n2.receive_->data[1] = 0x44;

    bus.loop();

    assertEqual(n1.send_count_, 0);
    assertEqual(n2.send_count_, 0);
    assertEqual(n3.send_count_, 3);
    assertTrue(frameEquals(*n1.receive_, *n3.send_[0]));
    assertTrue(frameEquals(*n1.receive_, *n3.send_[1]));
    assertTrue(frameEquals(*n2.receive_, *n3.send_[2]));

}

#endif  // __R51_TESTS_TEST_BUS__
