#ifndef __R51_TESTS_MOCK_YIELD__
#define __R51_TESTS_MOCK_YIELD__

#include <Canny.h>
#include <Caster.h>

#include "src/debug.h"
#include "src/events.h"


// Mock yield implementation for node tests.
class MockYield {
    public:
        MockYield(int capacity = 1, uint32_t filter_id = 0, uint32_t filter_mask = 0xFFFFFFFF) :
            impl(this), capacity_(capacity), count_(0),
            filter_id_(filter_id), filter_mask_(filter_mask),
            frames_(new Canny::Frame[capacity_]) {}

        ~MockYield() {
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

        class YieldImpl : public Caster::Yield<Message> {
            public:
                void operator()(const Message& msg) const override {
                    if (msg.type() != Message::CAN_FRAME) {
                        return;
                    }
                    if (mock_->filter_id_ == 0 || mock_->filter_id_ == (msg.can_frame().id() & mock_->filter_mask_)) {
                        mock_->append(msg.can_frame());
                    }
                }

            private:
                YieldImpl(MockYield* mock) : mock_(mock) {}
                MockYield* mock_;
                friend class MockYield;
        };

        YieldImpl impl;

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

#endif  // __R51_TESTS_MOCK_YIELD__
