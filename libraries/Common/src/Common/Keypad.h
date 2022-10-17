#ifndef _R51_COMMON_KEYPAD_H_
#define _R51_COMMON_KEYPAD_H_

#include <Arduino.h>
#include "Event.h"

namespace R51 {

enum class LEDMode : uint8_t {
    OFF         = 0,
    ON          = 1,
    BLINK       = 2,
    ALT_BLINK   = 3,
};

enum class LEDColor : uint8_t {
    WHITE   = 0,
    RED     = 1,
    GREEN   = 2,
    BLUE    = 3,
    CYAN    = 4,
    YELLOW  = 5,
    MAGENTA = 6,
    AMBER   = 7,
};

enum class KeypadEvent : uint8_t {
    KEY_STATE       = 0x01, // Send on button press/release.
    ENCODER_STATE   = 0x02, // Send on rotary encoder rotation.

    INDICATOR_CMD   = 0x11, // Change power or color of the indicator on a
                            // single key/encoder.
    BRIGHTNESS_CMD  = 0x12, // Change the brightness of the indicator LEDs for
                            // the entire keypad.
    BACKLIGHT_CMD   = 0x13, // Change the color and brightness of the keypad's
                            // backlight.
};

// Sent when a key is pressed or released.
class KeyState : public Event {
    public:
        KeyState(uint8_t keypad = 0x00, uint8_t key = 0x00, bool pressed = false) :
            Event(
                SubSystem::KEYPAD,
                (uint8_t)KeypadEvent::KEY_STATE,
                {keypad, key, pressed}) {}

        // The ID of the keypad. This is assigned to the Keypad object on construction.
        EVENT_PROPERTY(uint8_t, keypad, data[0], data[0] = value);
        // The ID of the key being pressed. This is unique among keys on the
        // keypad.
        EVENT_PROPERTY(uint8_t, key, data[1], data[1] = value);
        // True if the key is being pressed. False if it is not.
        EVENT_PROPERTY(bool, pressed, (bool)data[2], data[2] = (uint8_t)value);
};

// Sent when a rotary encoder is turned.
class EncoderState : public Event {
    public:
        EncoderState() : Event(SubSystem::KEYPAD,
                (uint8_t)KeypadEvent::ENCODER_STATE,
                {0x00, 0x00, 0x00}) {}

        // The ID of the keypad. This is assigned to the Keypad object on construction.
        EVENT_PROPERTY(uint8_t, keypad, data[0], data[0] = value);
        // The ID of the encoder being rotated. This is unique among encoders
        // on the keypad.
        EVENT_PROPERTY(uint8_t, encoder, data[1], data[1] = value);
        // The change in position of the encoder. Positive is clockwise,
        // negative is counter-clockwise.
        EVENT_PROPERTY(int8_t, delta, (int8_t)data[2], data[2] = (uint8_t)value);
};

// Command to change the color of a key's indicator LED.
class IndicatorCommand : public Event {
    public:
        IndicatorCommand(uint8_t keypad = 0xFF) :
                Event(SubSystem::KEYPAD, (uint8_t)KeypadEvent::INDICATOR_CMD,
                        {keypad, 0xFF, 0x00, 0x00, 0x00}) {}

        // The ID of the keypad.
        EVENT_PROPERTY(uint8_t, keypad, data[0], data[0] = value);
        // The ID of the indicator LED. This is unique among indicator LEDs on
        // the keypad and is generally associated with a specific key or
        // encoder.
        EVENT_PROPERTY(uint8_t, led, data[1], data[1] = value);
        // The power mode of the LED.
        EVENT_PROPERTY(LEDMode, mode, (LEDMode)data[2], data[2] = (uint8_t)value);
        // The color of the LED.
        EVENT_PROPERTY(LEDColor, color,
                (LEDColor)data[3], data[3] = (uint8_t)value);
        // The alternate color of the LEd in alt blink mode.
        EVENT_PROPERTY(LEDColor, alt_color,
                (LEDColor)data[4], data[4] = (uint8_t)value);
};

// Command to change the brightness of the indicator LEDs.
class BrightnessCommand : public Event {
    public:
        BrightnessCommand(uint8_t keypad = 0xFF) :
                Event(SubSystem::KEYPAD, (uint8_t)KeypadEvent::BRIGHTNESS_CMD,
                        {keypad, 0x00}) {}

        // The ID of the keypad.
        EVENT_PROPERTY(uint8_t, keypad, data[0], data[0] = value);
        // The brightness for the backlight from 0 to 256. 0 turns off the LED.
        EVENT_PROPERTY(uint8_t, brightness, data[1], data[1] = value);
};

// Command to change the color and brightness of the keypad backlight. For
// keypads without an independent backlight this will be set when the key is
// not illuminated.
class BacklightCommand : public Event {
    public:
        BacklightCommand(uint8_t keypad = 0xFF) :
                Event(SubSystem::KEYPAD, (uint8_t)KeypadEvent::BACKLIGHT_CMD,
                        {keypad, 0x00}) {}

        // The ID of the keypad.
        EVENT_PROPERTY(uint8_t, keypad, data[0], data[0] = value);
        // The brightness for the backlight from 0 to 256. 0 turns off the LED.
        EVENT_PROPERTY(uint8_t, brightness, data[1], data[1] = value);
        // The color of the backlight.
        EVENT_PROPERTY(LEDColor, color, (LEDColor)data[2], data[2] = (uint8_t)value);
};

}  // namespace R51

#endif  // _R51_COMMON_KEYPAD_H_
