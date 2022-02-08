#ifndef __R51_TESTS_MOCK_BROADCAST__
#define __R51_TESTS_MOCK_BROADCAST__

#include "src/bus.h"


// Mock broadcast implementation for node tests.
class MockBroadcast {
    public:
        MockBroadcast(int capacity) : impl(this), capacity_(capacity), count_(0), frames_(new Frame[capacity_]()) {}

        ~MockBroadcast() {
            delete frames_;
        }

        void reset() {
            count_ = 0;
        }

        Frame* frames() const {
            return frames_;
        }

        int count() const {
            return count_;
        }

        class BroadcastImpl : public Broadcast {
            public:
                void operator()(const Frame& frame) const override {
                    mock_->append(frame);
                }

            private:
                BroadcastImpl(MockBroadcast* mock) : mock_(mock) {}
                MockBroadcast* mock_;
                friend class MockBroadcast;
        };

        BroadcastImpl impl;

    private:
        void append(const Frame& frame) {
            if (count_ < capacity_) {
                copyFrame(&frames_[count_], frame);
            }
            ++count_;
        }

        int capacity_;
        int count_;
        Frame* frames_;
};

#endif  // __R51_TESTS_MOCK_BROADCAST__
