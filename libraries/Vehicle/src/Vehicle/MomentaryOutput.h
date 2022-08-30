#ifndef __R51_VEHICLE_MOMENTARY_OUTPUT__
#define __R51_VEHICLE_MOMENTARY_OUTPUT__

#include <Arduino.h>
#include <Faker.h>

namespace R51 {

// Momentarily enable a digital output pin for a set amount of time. Most
// useful for driving external systems which use momentary push button inputs.
class MomentaryOutput {
    public:
        // Create a new momentary trigger which controls the given pin. The pin
        // should be configured as a digital output prior to calling this. When
        // triggered the pin is held high for trigger_ms. The pin cannot be
        // triggered for cooldown_ms after trigger_ms expires. By default
        // cooldown_ms is set to trigger_ms. By default the pin is driven high
        // when triggered .The high param can be set to false to drive the pin
        // low instead.
        MomentaryOutput(int pin, uint16_t trigger_ms, 
                Faker::Clock* clock = Faker::Clock::real(),
                Faker::GPIO* gpio = Faker::GPIO::real());
        MomentaryOutput(int pin, uint16_t trigger_ms, int32_t cooldown_ms = -1, bool high = true,
                Faker::Clock* clock = Faker::Clock::real(),
                Faker::GPIO* gpio = Faker::GPIO::real());

        // Update the state of the pin. Must be called in the main loop.
        void update();

        // Trigger the pin. Return true on success or false if the pin is
        // currently triggered or in the cooldown phase.
        bool trigger();

    private:
        Faker::Clock* clock_;
        Faker::GPIO* gpio_;
        int pin_;
        bool high_;
        bool triggered_;
        uint16_t trigger_ms_;
        uint32_t trigger_time_;
        uint16_t cooldown_ms_;
};

}  // namespace R51

#endif  // __R51_VEHICLE_MOMENTARY_OUTPUT__
