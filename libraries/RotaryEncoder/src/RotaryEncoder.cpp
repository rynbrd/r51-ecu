#include "RotaryEncoder.h"

#include <Arduino.h>
#include <Caster.h>
#include <Core.h>

namespace R51 {
namespace {

using ::Caster::Yield;

uint32_t toNeopixelColor(LEDColor color) {
    switch (color) {
        case LEDColor::WHITE:
            return 0xFFFFFF;
        case LEDColor::RED:
            return 0xFF0000;
        case LEDColor::GREEN:
            return 0x00FF00;
        case LEDColor::BLUE:
            return 0x0000FF;
        case LEDColor::CYAN:
            return 0x00FFFF;
        case LEDColor::YELLOW:
            return 0xFFFF00;
        case LEDColor::MAGENTA:
            return 0xFF00FF;
        case LEDColor::AMBER:
            return 0xFF4000;
        default:
            return 0x000000;
    }
}

}  // namespace

bool RotaryEncoder::begin(uint8_t addr) {
    if (!encoder_.begin(addr) || !neopixel_.begin(addr)) {
        return false;
    }
    if (irq_pin_ > -1) {
        pinMode(irq_pin_, INPUT);
    }
    encoder_.pinMode(kSwitchPin, INPUT_PULLUP);
    delay(10);
    encoder_.setGPIOInterrupts((uint32_t)1 << kSwitchPin, 1);
    encoder_.enableEncoderInterrupt();
    showPixel();
    return true;
}

int8_t RotaryEncoder::getDelta() {
    pos_ = new_pos_;
    new_pos_ = encoder_.getEncoderPosition();
    return pos_ - new_pos_;
}

int8_t RotaryEncoder::getSwitch() {
    sw_ = new_sw_;
    new_sw_ = !encoder_.digitalRead(kSwitchPin);
    if (new_sw_ != sw_) {
        return new_sw_ ? 1 : -1;
    }
    return 0;
}

void RotaryEncoder::setColor(LEDMode mode, LEDColor color) {
    switch (mode) {
        case LEDMode::ON:
            color_ = toNeopixelColor(color);
            break;
        default:
        case LEDMode::OFF:
            color_ = 0x000000;
            break;
    }
}

void RotaryEncoder::setBrightness(uint8_t value) {
    brightness_ = value;
}

void RotaryEncoder::setBacklight(LEDColor color, uint8_t brightness) {
    backlight_color_ = toNeopixelColor(color);
    backlight_brightness_ = brightness;
}

void RotaryEncoder::showPixel() {
    if (brightness_ == 0 || color_ == 0x000000) {
        if (backlight_brightness_ == 0) {
            neopixel_.setBrightness(backlight_brightness_);
            setPixelColor(0x000000);
        } else {
            neopixel_.setBrightness(backlight_brightness_);
            setPixelColor(backlight_color_);
        }
    } else {
        neopixel_.setBrightness(brightness_);
        setPixelColor(color_);
    }
    neopixel_.show();
}

int RotaryEncoder::getIRQPin() const {
    return irq_pin_;
}

void RotaryEncoder::setPixelColor(uint32_t color) {
    neopixel_.setPixelColor(0, neopixel_.Color(
                (color & 0xFF0000) >> 16,
                (color & 0x00FF00) >> 8,
                (color & 0x0000FF)));
}

RotaryEncoderGroup::RotaryEncoderGroup(uint8_t keypad, RotaryEncoder** encoders, uint8_t count) :
        encoders_(encoders), count_(count) {
    encoder_event_.keypad(keypad);
    keypress_event_.keypad(keypad);
}

void RotaryEncoderGroup::handle(const Message& msg, const Yield<Message>&) {
    if (msg.type() != Message::EVENT ||
            msg.event()->subsystem != (uint8_t)SubSystem::KEYPAD) {
        return;
    }
    switch ((KeypadEvent)msg.event()->id) {
        case KeypadEvent::INDICATOR_CMD:
            handleIndicatorCommand((IndicatorCommand*)msg.event());
            break;
        case KeypadEvent::BRIGHTNESS_CMD:
            handleBrightnessCommand((BrightnessCommand*)msg.event());
            break;
        case KeypadEvent::BACKLIGHT_CMD:
            handleBacklightCommand((BacklightCommand*)msg.event());
            break;
        default:
            break;
    }
}

void RotaryEncoderGroup::handleIndicatorCommand(const IndicatorCommand* cmd) {
    if (cmd->keypad() != keypress_event_.keypad() || cmd->led() >= count_) {
        return;
    }
    RotaryEncoder* encoder = encoders_[cmd->led()];
    encoder->setColor(cmd->mode(), cmd->color());
    encoder->showPixel();
}

void RotaryEncoderGroup::handleBrightnessCommand(const BrightnessCommand* cmd) {
    if (cmd->keypad() != keypress_event_.keypad()) {
        return;
    }
    for (uint8_t i = 0; i < count_; ++i) {
        encoders_[i]->setBrightness(cmd->brightness());
        encoders_[i]->showPixel();
    }
}

void RotaryEncoderGroup::handleBacklightCommand(const BacklightCommand* cmd) {
    if (cmd->keypad() != keypress_event_.keypad()) {
        return;
    }
    for (uint8_t i = 0; i < count_; ++i) {
        encoders_[i]->setBacklight(cmd->color(), cmd->brightness());
        encoders_[i]->showPixel();
    }
}

void RotaryEncoderGroup::emit(const Yield<Message>& yield) {
    RotaryEncoder* encoder;
    int8_t sw;
    for (uint8_t i = 0; i < count_; ++i) {
        encoder = encoders_[i];

        if (encoder->getIRQPin() != -1 && digitalRead(encoder->getIRQPin()) != LOW) {
            continue;
        }

        sw = encoder->getSwitch();
        encoder_event_.delta(encoder->getDelta());

        if (sw != 0) {
            keypress_event_.key(i);
            keypress_event_.pressed(sw == 1);
            yield(MessageView(&keypress_event_));
        }
        if (encoder_event_.delta() != 0) {
            encoder_event_.encoder(i);
            yield(MessageView(&encoder_event_));
        }
    }
}

}  // namespace R51
