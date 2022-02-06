#ifndef __R51_IO__
#define __R51_IO__

#include <Arduino.h>

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
        MomentaryPin(int pin, uint16_t trigger_ms, int32_t cooldown_ms = -1, bool high = true)
            : pin_(pin), high_(high), triggered_(false), 
              trigger_ms_(trigger_ms), trigger_time_(0),
              cooldown_ms_(cooldown_ms < 0 ? trigger_ms : (uint16_t)cooldown_ms) {}

        // Loop must be called for each iteration of the main loop.
        void loop();

        // Trigger the pin. Return true on success or false if the pin is
        // currently triggered or in the cooldown phase.
        bool trigger();

    private:
        int pin_;
        bool high_;
        bool triggered_;
        uint16_t trigger_ms_;
        uint32_t trigger_time_;
        uint16_t cooldown_ms_;
};

#endif  // __R51_IO__
