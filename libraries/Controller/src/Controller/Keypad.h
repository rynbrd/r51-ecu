#ifndef _R51_CONTROLLER_KEYPAD_H_
#define _R51_CONTROLLER_KEYPAD_H_

#include <Arduino.h>
#include <Canny.h>
#include <Common.h>

namespace R51 {

enum class KeypadColor : uint8_t {
    OFF     = 0,
    WHITE   = 1,
    RED     = 2,
    GREEN   = 3,
    BLUE    = 4,
    CYAN    = 5,
    YELLOW  = 6,
    MAGENTA = 7,
    AMBER   = 8,
};

enum class KeypadEvent : uint8_t {
    KEY_PRESS_EVENT         = 0x01, // Send on button press/release.
    ENCODER_ROTATE_EVENT    = 0x02, // Send on rotary encoder rotation.

    INDICATOR_LED_CMD       = 0x10, // Change power state, color, and
                                    // brightness of the indicator LED on a
                                    // key or encoder.
    BACKLIGHT_LED_CMD       = 0x11, // Change power state, color, and
                                    // brightness of the backlight on a key or
                                    // encoder.
};

// Sent when a key is pressed or released.
class KeyPressEvent : public Event {
    public:
        KeyPressEvent() : Event(SubSystem::KEYPAD,
                (uint8_t)KeypadEvent::KEY_PRESS_EVENT,
                {0x00, 0x00, 0x00}) {}

        // The ID of the keypad. This is assigned to the Keypad object on construction.
        EVENT_PROPERTY(uint8_t, keypad, data[0], data[0] = value);
        // The ID of the key being pressed. This is unique among keys and
        // encoders on the keypad.
        EVENT_PROPERTY(uint8_t, id, data[1], data[1] = value);
        // True if the key is being pressed. False if it is not.
        EVENT_PROPERTY(bool, pressed, (bool)data[2], data[2] = (uint8_t)value);
};

// Sent when a rotary encoder is turned.
class EncoderRotateEvent : public Event {
    public:
        EncoderRotateEvent() : Event(SubSystem::KEYPAD,
                (uint8_t)KeypadEvent::ENCODER_ROTATE_EVENT,
                {0x00, 0x00}) {}

        // The ID of the keypad. This is assigned to the Keypad object on construction.
        EVENT_PROPERTY(uint8_t, keypad, data[0], data[0] = value);
        // The ID of the encoder being rotated. This is unique among keys and
        // encoders on the keypad.
        EVENT_PROPERTY(uint8_t, id, data[1], data[1] = value);
        // The change in position of the encoder. Positive is clockwise,
        // negative is counter-clockwise.
        EVENT_PROPERTY(int8_t, delta, (int8_t)data[2], data[2] = (uint8_t)value);
};

class LEDCommand : public Event {
    public:
        LEDCommand(KeypadEvent event) :
                Event(SubSystem::KEYPAD, (uint8_t)event, {0x00, 0x00}) {}

        // The ID of the keypad.
        EVENT_PROPERTY(uint8_t, keypad, data[0], data[0] = value);
        // The ID of the key or encoder to set the LED value on.
        EVENT_PROPERTY(uint8_t, id, data[1], data[1] = value);
        // The color the LED. Off is a color.
        EVENT_PROPERTY(KeypadColor, color, (KeypadColor)data[2], data[2] = (uint8_t)value);
        // The brightness for the LED from 0 to 256. 0 turns off the LED.
        EVENT_PROPERTY(uint8_t, brightness, data[3], data[3] = value);

};

class IndicatorLEDCommand : public LEDCommand {
    public:
        IndicatorLEDCommand() : LEDCommand(KeypadEvent::BACKLIGHT_LED_CMD) {}
};

class BacklightLEDCommand : public LEDCommand {
    public:
        BacklightLEDCommand() : LEDCommand(KeypadEvent::BACKLIGHT_LED_CMD) {}
};

}  // namespace R51 {

#endif  // _R51_CONTROLLER_KEYPAD_H_
