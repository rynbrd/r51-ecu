#ifndef __R51_STEERING__
#define __R51_STEERING__

#include "AnalogMultiButton.h"
#include "bus.h"
#include "clock.h"
#include "gpio.h"


// Steering wheel keypad. Sends 0x5800 CAN frames on button press and release.
//
// Frame 0x5800: Steering Keypad State Frame
//   Byte 0: Button State
//     Bit 0: Power
//     Bit 1: Mode
//     Bit 2: Volume Up
//     Bit 3: Volume Down
//     Bit 4: Seek Up
//     Bit 5: Seek Down
//     Bit 6-7: unused
//   Byte 1-7: unused
//
// Bits are set to 1 when a button on the keypad is held down and 0 when
// released.
class SteeringKeypad : public Node {
    public:
        // Construct a new steering switch keypad object.
        // analog pins to communicate. See config.h for configuration.
        SteeringKeypad(Clock* clock = Clock::real(), GPIO* gpio = GPIO::real());

        // Free resources associated with the keypad.
        ~SteeringKeypad() override;

        // Broadcast a frame on keypad state change.
        void receive(const Broadcast& broadcast) override;

        // Noop. This node does not process frames.
        void send(const Frame&) override {}

        // Always returns false. This node does not process frames.
        bool filter(uint32_t) override { return false; }

    private:
        uint32_t last_change_;
        Clock* clock_;
        Frame frame_;
        AnalogMultiButton* sw_a_;
        AnalogMultiButton* sw_b_;
};

#endif  // __R51_STEERING__
