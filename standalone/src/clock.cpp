#include "clock.h"


// Unshasow Arduino functions.
inline uint32_t arduino_millis() { return millis(); }
inline void arduino_delay(uint32_t ms) { delay(ms); }

class RealClock : public Clock {
    public:
        uint32_t millis() override {
            return arduino_millis();
        }

        void delay(uint32_t ms) override {
            arduino_delay(ms);
        }
};

RealClock real_clock;

Clock* Clock::real() {
    return &real_clock;
}
