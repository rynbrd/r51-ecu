#ifndef __R51_KEYPAD__
#define __R51_KEYPAD__

#include "controller.h"

// Used to send keypress events somplace.
class KeypadController : public Controller {
    public:
        // Buttons available to send events for.
        enum Button {
            POWER,
            MODE,
            VOLUME_UP,
            VOLUME_DOWN,
            SEEK_UP,
            SEEK_DOWN,
        };

        // Default constructor and destructor.
        KeypadController() {}
        virtual ~KeypadController() {}

        // Send a button press.
        virtual void press(Button button) = 0;

        // Send a button release.
        virtual void release(Button button) = 0;

        // Push state changes.
        virtual void push() = 0;
};

#endif  // __R51_KEYPAD__
