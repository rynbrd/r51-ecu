#ifndef __R51_TESTS_MOCK_BROADCAST__
#define __R51_TESTS_MOCK_BROADCAST__

#include <Canny.h>

#include "src/bus.h"
#include "src/debug.h"


// Mock broadcast implementation for node tests.
class MockBroadcast {
    public:
        MockBroadcast(int capacity = 1, uint32_t filter_id = 0, uint32_t filter_mask = 0xFFFFFFFF) :
            impl(this), capacity_(capacity), count_(0),
            filter_id_(filter_id), filter_mask_(filter_mask),
            frames_(new Canny::Frame[capacity_]) {}

        ~MockBroadcast() {
            delete[] frames_;
        }

        Canny::Frame* frames() const {
            return frames_;
        }

        int count() const {
            return count_;
        }

        void filter(uint32_t id, uint32_t mask = 0xFFFFFFFF) {
            filter_id_ = id;
            filter_mask_ = mask;
        }

        void reset() {
            count_ = 0;
        }

        class BroadcastImpl : public Broadcast {
            public:
                void operator()(const Canny::Frame& frame) const override {
                    if (mock_->filter_id_ == 0 || mock_->filter_id_ == (frame.id() & mock_->filter_mask_)) {
                        mock_->append(frame);
                    }
                }

            private:
                BroadcastImpl(MockBroadcast* mock) : mock_(mock) {}
                MockBroadcast* mock_;
                friend class MockBroadcast;
        };

        BroadcastImpl impl;

    private:
        void append(const Canny::Frame& frame) {
            if (count_ < capacity_) {
                frames_[count_] = Canny::Frame(frame);
            }
            ++count_;
        }

        int capacity_;
        int count_;
        uint32_t filter_id_;
        uint32_t filter_mask_;
        Canny::Frame* frames_;
};

#endif  // __R51_TESTS_MOCK_BROADCAST__
