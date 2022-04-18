#ifndef __R51_STEERING__
#define __R51_STEERING__

#include <Canny.h>
#include <Caster.h>
#include <Faker.h>

#include "AnalogMultiButton.h"
#include "events.h"


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
class SteeringKeypad : public Caster::Node<Canny::Frame> {
    public:
        // Construct a new steering switch keypad object.
        // analog pins to communicate. See config.h for configuration.
        SteeringKeypad(Faker::Clock* clock = Faker::Clock::real(),
                Faker::GPIO* gpio = Faker::GPIO::real());

        // Noop. This node does not process frames.
        void handle(const Canny::Frame&) override {}

        // Emit a state frame on keypad state change.
        void emit(const Caster::Yield<Canny::Frame>& yield) override;

    private:
        uint32_t last_change_;
        Faker::Clock* clock_;
        Canny::Frame frame_;
        AnalogMultiButton sw_a_;
        AnalogMultiButton sw_b_;
};

#endif  // __R51_STEERING__
