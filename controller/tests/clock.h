#ifndef __R51_TESTS_CLOCK__
#define __R51_TESTS_CLOCK__

#include "src/clock.h"


class MockClock : public Clock {
    public:
        MockClock() : millis_(0) {}

        // Return the current mocked time.
        uint32_t millis() override {
            return millis_;
        }

        // Mock a delay. Advances time by ms and returns immediately.
        void delay(uint32_t ms) override {
            millis_ += ms;
        }

        // Set the clock to a specific time.
        void set(uint32_t millis) {
            millis_ = millis;
        } 

    private:
        uint32_t millis_;
};

#endif  // __R51_TESTS_CLOCK__
