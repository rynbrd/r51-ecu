#ifndef _R51_CONTROLLER_BUTTONS_H_
#define _R51_CONTROLLER_BUTTONS_H_

#include <Arduino.h>
#include <Faker.h>
#include <Foundation.h>

namespace R51 {

// A button that repeats while it's held.
class RepeatButton {
    public:
        RepeatButton(uint32_t interval, Faker::Clock* clock = Faker::Clock::real()) :
            ticker_(interval, true, clock) {}

        // Press the button.
        void press();

        // Return true if the button's repeat action should be executed.
        bool trigger();

        // Release the button. Returns true if its action should be triggered.
        // This occurs when the button does not exceed its repeat interval.
        bool release();

    private:
        Ticker ticker_;
};

// A button that may perform a separate action on long press.
class LongPressButton {
    public:
        LongPressButton(uint32_t timeout, Faker::Clock* clock = Faker::Clock::real()) :
            clock_(clock), timeout_(timeout), pressed_(0), state_(0) {}

        // Press the button.
        void press();

        // Return true if the button's long press action should be executed.
        bool trigger();

        // Release the button. Returns true if the button's short press action
        // should be executed.
        bool release();

    private:
        Faker::Clock* clock_;
        uint32_t timeout_;
        uint32_t pressed_;
        uint8_t state_;
};

}  // namespace R51

#endif  // _R51_CONTROLLER_BUTTONS_H_
