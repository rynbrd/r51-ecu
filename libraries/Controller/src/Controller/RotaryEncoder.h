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
                color_(0xFFFFFF), brightness_(0),
                backlight_color_(0xFFFFFF), backlight_brightness_(0),
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
        void setColor(KeypadColor color);

        // Set the brightness of the encoder's neopixel. Must call showPixel
        // for the changes to take effect.
        void setBrightness(uint8_t value);

        // Set the color and brightness of the encoder's backlight. A backlight
        // is simulated by turning on the neopixel with this color and
        // brightness when setColor or setBrightness are turned "off". Must
        // call showPixel for the changes to take effect.
        void setBacklight(KeypadColor color, uint8_t brightness);

        // Update the neopixel with the new color and brightness.
        void showPixel();

    private:
        Adafruit_seesaw encoder_;
        seesaw_NeoPixel neopixel_;

        uint32_t color_;
        uint8_t brightness_;

        uint32_t backlight_color_;
        uint8_t backlight_brightness_;

        int32_t pos_;
        int32_t new_pos_;
        bool sw_;
        bool new_sw_;

        static const int kSwitchPin = 24;
};

// A bus node that manages a group of Adafruit rotary encoders. Encoders have
// two inputs - the encoder knob and the press switch of the encoder. IDs are
// sequentually assigned to each encoder knob and switch starting at 0.
// Encoders also have a backlight that can be controlled via the
// BACKLIGHT_LED_CMD event. A maximum of 8 encoders are supported in the group.
// This is, without coincidence, also the maximum number of encoders that can
// be connected to a single I2C bus.
class RotaryEncoderGroup : public Caster::Node<Message> {
    public:
        // Construct a new group with the given keypad ID and set of encoders.
        RotaryEncoderGroup(uint8_t keypad, RotaryEncoder** encoders, uint8_t count);

        // Handle a backlight LED command.
        void handle(const Message& msg, const Caster::Yield<Message>&) override;

        // Emit encoder and keypad events.
        void emit(const Caster::Yield<Message>& yield) override;

        // Enable interrupts and set the interrupts callbacks. When interrupts
        // are enabled the ISR must call interrupt() in order for data to be
        // read from an encoder. The callbacks are used to pause and resume
        // interrupts to avoid hanging the I2C bus. 
        void enableInterrupts(void (*pause_cb)(uint8_t), void (*resume_cb)(uint8_t));

        // Disable interrupts. Emit will read events on each iteration instead
        // of waiting for an interrupt.
        void disableInterrupts();

        // Called by an ISR to trigger a read for a specific encoder. If n is
        // 0xFF then all encoders are read.
        void interrupt(uint8_t n);
    private:

        void handleIndicatorCommand(const IndicatorCommand* cmd);
        void handleBrightnessCommand(const BrightnessCommand* cmd);
        void handleBacklightCommand(const BacklightCommand* cmd);

        void pauseInterrupts(uint8_t n);
        void resumeInterrupts(uint8_t n);

        EncoderRotateEvent encoder_event_;
        KeyPressEvent keypress_event_;

        uint8_t keypad_;
        RotaryEncoder** encoders_;
        uint8_t count_;

        bool intr_enable_;
        volatile uint8_t intr_read_;
        void (*pause_intr_cb_)(uint8_t);
        void (*resume_intr_cb_)(uint8_t);
};

}  // namespace R51 {

#endif  // _R51_CONTROLLER_ROTARY_ENCODER_H_
