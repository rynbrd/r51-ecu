#ifndef __R51_STEERING__
#define __R51_STEERING__

#include <AnalogMultiButton.h>

#include "config.h"
#include "keypad.h"

// Steering wheel switch control. Sends CAN frames to the dash on button press
// and release.
class SteeringSwitch {
    public:
        // Construct a new steering switch object that uses the two given
        // analog pins to communicate.
        SteeringSwitch(int sw_a_pin, int sw_b_pin);

        virtual ~SteeringSwitch();

        // Connect the steering switch to a keypad controller.
        void connect(KeypadController* controller);

        // Read button events and push them to the dash audio controller.
        void read();

    private:
        AnalogMultiButton* sw_a_;
        AnalogMultiButton* sw_b_;
        KeypadController* controller_;
};

#endif  // __R51_STEERING__
