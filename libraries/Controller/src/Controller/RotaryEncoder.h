#ifndef _R51_CONTROLLER_ROTARY_ENCODER_H_
#define _R51_CONTROLLER_ROTARY_ENCODER_H_

#include <Adafruit_seesaw.h>
#include <Arduino.h>
#include <Canny.h>
#include <Common.h>
#include <seesaw_neopixel.h>
#include "Keypad.h"

namespace R51 {

// A single Adafruit Seesaw rotary encoder. All methods rely on hardware
// interrupts and are not safe to execute concurrently with user defined
// interrupts. Interrupts should be detached while calling methods on this
// class.
class RotaryEncoder {
    public:
        RotaryEncoder(TwoWire* wire) : encoder_(wire),
                neopixel_(1, 6, NEO_GRB + NEO_KHZ800, wire),
                pos_(0), new_pos_(0), sw_(false), new_sw_(false) {}

        // Connect to the encoder on the given I2C address.
        bool begin(uint8_t addr);

        // Return the change in position of the encoder since last check.
        int8_t getDelta();

        // Return 1 if the switch was pressed, -1 if it was released, and 0 if
        // no change occurred.
        int8_t getSwitch();

        // Set the color of the encoder's neopixel. Must call showPixel for the
        // changes to take effect.
        void setColor(uint8_t r, uint8_t g, uint8_t b);

        // Set the brightness of the encoder's neopixel. Must call showPixel
        // for the changes to take effect.
        void setBrightness(uint8_t value);

        // Update the neopixel with the new color and brightness.
        void showPixel();

    private:
        Adafruit_seesaw encoder_;
        seesaw_NeoPixel neopixel_;

        int32_t pos_;
        int32_t new_pos_;
        bool sw_;
        bool new_sw_;

        static const int kSwitchPin = 24;
};

class RotaryEncoderGroup : public Caster::Node<Message> {
    public:
        RotaryEncoderGroup(uint8_t keypad, RotaryEncoder** encoders, uint8_t count);

        void handle(const Message& msg, const Caster::Yield<Message>& yield) override;

        void emit(const Caster::Yield<Message>& yield) override;

    private:
        void setBacklightColor(RotaryEncoder* encoder, KeypadColor color);
        void pauseInterrupts(uint8_t n);
        void resumeInterrupts(uint8_t n);

        EncoderRotateEvent encoder_event_;
        KeyPressEvent keypress_event_;

        uint8_t keypad_;
        RotaryEncoder** encoders_;
        uint8_t count_;

        void (*pause_int_cb_)(uint8_t);
        void (*resume_int_cb_)(uint8_t);
};

}  // namespace R51 {

#endif  // _R51_CONTROLLER_ROTARY_ENCODER_H_
