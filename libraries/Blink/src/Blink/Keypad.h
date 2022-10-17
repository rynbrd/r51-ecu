#ifndef _R51_BLINK_KEYPAD_H_
#define _R51_BLINK_KEYPAD_H_

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include <Common.h>

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
        BlinkKeypad(uint8_t address, uint8_t keypad, uint8_t key_count);

        // Initialize the button LED state.
        void init(const Caster::Yield<Message>& yield) override;

        // Handle J1939 keypad message and a LED command events. Keypad events
        // are yield'd in response to J1939 keypad messages.
        void handle(const Message& msg, const Caster::Yield<Message>& yield) override;

    private:
        void handleJ1939Claim(const J1939Claim& claim);
        void handleJ1939Message(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleIndicatorCommand(const IndicatorCommand* cmd,
                const Caster::Yield<Message>& yield);
        void handleBrightnessCommand(const BrightnessCommand* cmd,
                const Caster::Yield<Message>& yield);
        void handleBacklightCommand(const BacklightCommand* cmd,
                const Caster::Yield<Message>& yield);

        void setKeyLED(const Caster::Yield<Message>& yield, uint8_t key, LEDMode mode,
                LEDColor color = LEDColor::WHITE,
                LEDColor alt_color = LEDColor::WHITE);
        void setKeyBrightness(const Caster::Yield<Message>& yield, uint8_t brightness);
        void setBacklightColor(const Caster::Yield<Message>& yield, LEDColor color);
        void setBacklightBrightness(const Caster::Yield<Message>& yield, uint8_t brightness);

        KeyState keypress_;
        Canny::J1939Message command_;

        uint8_t key_count_;
};

}  // namespace R51

#endif  // _R51_BLINK_KEYPAD_H_
