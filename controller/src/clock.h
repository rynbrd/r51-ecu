#ifndef __R51_CLOCK__
#define __R51_CLOCK__

#include <Arduino.h>


// Base clock interface. Allows the clock to be mocked.
class Clock {
    public:
        // Return a real clock.
        static Clock* real();

        Clock() = default;
        virtual ~Clock() = default;

        // Return the number of milliseconds since the Arduino started.
        virtual uint32_t millis() = 0;

        // Pause the Arduino for the given number of milliseconds.
        virtual void delay(uint32_t) = 0;
};

#endif  // __R51_CLOCK__
