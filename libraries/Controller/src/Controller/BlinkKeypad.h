#ifndef _R51_CONTROLLER_BLINK_KEYPAD_H_
#define _R51_CONTROLLER_BLINK_KEYPAD_H_

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include <Common.h>
#include "Keypad.h"

namespace R51 {

// A bus node that manages a Blink Marine PKP keypad over J1939. PKP keys are 1
// indexed. This node emits events that are 0 indexed so adjust expectations
// accordingly.
class BlinkKeypad : public Caster::Node<Message> {
    public:
        // Construct a node that interacts the PKP keypad at the given address.
        // The keypad value identifies the keypad on the system and must be
        // unique to this specific keypad. The key count is the number of
        // buttons on the PKP.
        BlinkKeypad(uint8_t keypad, uint8_t address, uint8_t key_count);

        // Handle J1939 keypad message and a LED command events. Keypad events
        // are yield'd in response to J1939 keypad messages.
        void handle(const Message& msg, const Caster::Yield<Message>& yield) override;

        // Does nothing.
        void emit(const Caster::Yield<Message>&) override {}

    private:
        void handleJ1939Claim(const J1939Claim& claim);
        void handleJ1939Message(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleKeyLEDCommand(const KeyLEDCommand* cmd,
                const Caster::Yield<Message>& yield);
        void handleKeypadDimCommand(const KeypadDimCommand* cmd,
                const Caster::Yield<Message>& yield);

        KeyPressEvent keypress_;
        Canny::J1939Message command_;

        uint8_t address_;
        uint8_t key_count_;
};

}  // namespace R51

#endif  // _R51_CONTROLLER_BLINK_KEYPAD_H_
