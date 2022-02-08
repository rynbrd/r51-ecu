#ifndef __R51_IO__
#define __R51_IO__

#include <Arduino.h>

#include "clock.h"
#include "gpio.h"


// Momentarily enable a digital output pin for a set amount of time. Most
// useful for driving external systems which use momentary push button inputs.
class MomentaryPin {
    public:
        // Create a new momentary trigger which controls the given pin. The pin
        // should be configured as a digital output prior to calling this. When
        // triggered the pin is held high for trigger_ms. The pin cannot be
        // triggered for cooldown_ms after trigger_ms expires. By default
        // cooldown_ms is set to trigger_ms. By default the pin is driven high
        // when triggered .The high param can be set to false to drive the pin
        // low instead.
        MomentaryPin(int pin, uint16_t trigger_ms, int32_t cooldown_ms, bool high,
                Clock* clock = Clock::real(), GPIO* gpio = GPIO::real())
            : clock_(clock), gpio_(gpio),
              pin_(pin), high_(high), triggered_(false), 
              trigger_ms_(trigger_ms), trigger_time_(0),
              cooldown_ms_(cooldown_ms < 0 ? trigger_ms : (uint16_t)cooldown_ms) {}

        MomentaryPin(int pin, uint16_t trigger_ms, 
                Clock* clock = Clock::real(), GPIO* gpio = GPIO::real())
            : MomentaryPin(pin, trigger_ms, -1, true, clock, gpio) {}

        // Update the state of the pin. Must be called in the main loop.
        void update();

        // Trigger the pin. Return true on success or false if the pin is
        // currently triggered or in the cooldown phase.
        bool trigger();

    private:
        Clock* clock_;
        GPIO* gpio_;
        int pin_;
        bool high_;
        bool triggered_;
        uint16_t trigger_ms_;
        uint32_t trigger_time_;
        uint16_t cooldown_ms_;
};

#endif  // __R51_IO__
