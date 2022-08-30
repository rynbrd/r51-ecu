#ifndef _R51_COMMON_TICK_ 
#define _R51_COMMON_TICK_ 

#include <Arduino.h>
#include <Faker.h>

namespace R51 {

// Ticker is used to trigger actions repeatedly after a time inteval.
class Ticker {
    public:
        // Construct a ticker which triggers after interval ms have passed.
        explicit Ticker(uint32_t interval, Faker::Clock* clock = Faker::Clock::real());

        // Return true if the interval has passed. This will return true for
        // subsequent calls until reset is called.
        bool active() const;

        // Reset the ticker's interval.
        void reset();

        // Reste the ticker with a specific.
        void reset(uint32_t interval);

    private:
        uint32_t interval_;
        uint32_t last_tick_;
        Faker::Clock* clock_;
};

} // namespace R51

#endif  // _R51_COMMON_TICK_
