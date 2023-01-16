#ifndef _R51_ROTARY_ENCODER_H_
#define _R51_ROTARY_ENCODER_H_

#include <Adafruit_seesaw.h>
#include <Arduino.h>
#include <Canny.h>
#include <Core.h>
#include <seesaw_neopixel.h>

namespace R51 {

// A single Adafruit Seesaw rotary encoder.
class RotaryEncoder {
    public:
        // Construct a new rotary encoder connected over I2C. The specific I2C
        // connection must be provided with the wire arg. If the encoder's IRQ
        // pin is connected to a GPIO then that GPIO pin may be provided via
        // the irq_pin arg. The IRQ pin is driven low by the rotary encoder and
        // may be checked by the caller before calling getDelta or getSwitch.
        // It is used by RotaryEncoderGroup if set.
        RotaryEncoder(TwoWire* wire, int irq_pin=-1) : encoder_(wire),
                neopixel_(1, 6, NEO_GRB + NEO_KHZ800, wire),
                color_(0x000000), brightness_(0xFF),
                backlight_color_(0x000000), backlight_brightness_(0xFF),
                pos_(0), new_pos_(0), sw_(false), new_sw_(false),
                irq_pin_(irq_pin) {}

        // Connect to the encoder on the given I2C address.
        bool begin(uint8_t addr);

        // Return the change in position of the encoder since last check.
        int8_t getDelta();

        // Return 1 if the switch was pressed, -1 if it was released, and 0 if
        // no change occurred.
        int8_t getSwitch();

        // Set the color of the encoder's neopixel. Must call showPixel for the
        // changes to take effect.
        void setColor(LEDMode mode, LEDColor color);

        // Set the brightness of the encoder's neopixel. Must call showPixel
        // for the changes to take effect.
        void setBrightness(uint8_t value);

        // Set the color and brightness of the encoder's backlight. A backlight
        // is simulated by turning on the neopixel with this color and
        // brightness when setColor or setBrightness are turned "off". Must
        // call showPixel for the changes to take effect.
        void setBacklight(LEDColor color, uint8_t brightness);

        // Update the neopixel with the new color and brightness.
        void showPixel();

        // Return the GPIO pin that is connected to the rotary encoder's IRQ
        // pin. This pin is driven low by the rotary encoder board when data is
        // available to read. This pin may be read by the caller before calling
        // getDelta  and getSwitch.
        int getIRQPin() const;

    private:
        void setPixelColor(uint32_t color);

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

        int irq_pin_;

        static const int kSwitchPin = 24;
};

// A bus node that manages a group of Adafruit rotary encoders. Encoders have
// two inputs - the encoder knob and the press switch of the encoder. IDs are
// sequentually assigned to each encoder breakout such that the knob, switch,
// and LED all have the same ID. ID's are assigned starting at 0.
//
// A maximum of 8 encoders are supported in the group. This is, without
// coincidence, also the maximum number of encoders that can be connected to a
// single I2C bus.
class RotaryEncoderGroup : public Caster::Node<Message> {
    public:
        // Construct a new group with the given keypad ID and set of encoders.
        RotaryEncoderGroup(uint8_t keypad, RotaryEncoder** encoders, uint8_t count);

        // Handle a backlight LED command.
        void handle(const Message& msg, const Caster::Yield<Message>&) override;

        // Emit encoder and keypad events.
        void emit(const Caster::Yield<Message>& yield) override;
    private:
        void handleIndicatorCommand(const IndicatorCommand* cmd);
        void handleBrightnessCommand(const BrightnessCommand* cmd);
        void handleBacklightCommand(const BacklightCommand* cmd);

        EncoderState encoder_event_;
        KeyState keypress_event_;

        uint8_t keypad_;
        RotaryEncoder** encoders_;
        uint8_t count_;
};

}  // namespace R51 {

#endif  // _R51_ROTARY_ENCODER_H_
