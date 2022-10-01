#include "RotaryEncoder.h"

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>

namespace R51 {

using ::Caster::Yield;

bool RotaryEncoder::begin(uint8_t addr) {
    if (!encoder_.begin(addr) || !neopixel_.begin(addr)) {
        return false;
    }
    encoder_.pinMode(kSwitchPin, INPUT_PULLUP);
    delay(10);
    encoder_.setGPIOInterrupts((uint32_t)1 << kSwitchPin, 1);
    encoder_.enableEncoderInterrupt();
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

void RotaryEncoder::setColor(KeypadColor color) {
    switch (color) {
        case KeypadColor::WHITE:
            neopixel_.setPixelColor(0, neopixel_.Color(0xFF, 0xFF, 0xFF));
            break;
        case KeypadColor::RED:
            neopixel_.setPixelColor(0, neopixel_.Color(0xFF, 0x00, 0x00));
            break;
        case KeypadColor::GREEN:
            neopixel_.setPixelColor(0, neopixel_.Color(0x00, 0xFF, 0x00));
            break;
        case KeypadColor::BLUE:
            neopixel_.setPixelColor(0, neopixel_.Color(0x00, 0x00, 0xFF));
            break;
        case KeypadColor::CYAN:
            neopixel_.setPixelColor(0, neopixel_.Color(0x00, 0xFF, 0xFF));
            break;
        case KeypadColor::YELLOW:
            neopixel_.setPixelColor(0, neopixel_.Color(0xFF, 0xFF, 0x00));
            break;
        case KeypadColor::MAGENTA:
            neopixel_.setPixelColor(0, neopixel_.Color(0xFF, 0x00, 0xFF));
            break;
        case KeypadColor::AMBER:
            neopixel_.setPixelColor(0, neopixel_.Color(0xFF, 0xBF, 0x00));
            break;
        default:
            neopixel_.setPixelColor(0, neopixel_.Color(0x00, 0x00, 0x00));
            break;
    }
}

void RotaryEncoder::setBrightness(uint8_t value) {
    neopixel_.setBrightness(value);
}

void RotaryEncoder::showPixel() {
    neopixel_.show();
}

RotaryEncoderGroup::RotaryEncoderGroup(uint8_t keypad, RotaryEncoder** encoders, uint8_t count) :
        keypad_(keypad), encoders_(encoders), count_(count),
        intr_enable_(false), intr_read_(0),
        pause_intr_cb_(nullptr), resume_intr_cb_(nullptr) {
    encoder_event_.keypad(keypad_);
    keypress_event_.keypad(keypad_);
}

void RotaryEncoderGroup::handle(const Message& msg, const Yield<Message>&) {
    if (msg.type() != Message::EVENT ||
            msg.event().subsystem != (uint8_t)SubSystem::KEYPAD ||
            msg.event().id != (uint8_t)KeypadEvent::BACKLIGHT_LED_CMD) {
        return;
    }

    const auto* e = (BacklightLEDCommand*)&msg.event();
    uint8_t n = e->id() / 2;
    if (e->keypad() != keypad_ || n >= count_) {
        return;
    }
    RotaryEncoder* encoder = encoders_[n];
    pauseInterrupts(n);
    encoder->setBrightness(e->brightness());
    encoder->setColor(e->color());
    encoder->showPixel();
    resumeInterrupts(n);
}

void RotaryEncoderGroup::emit(const Yield<Message>& yield) {
    RotaryEncoder* encoder;
    int8_t sw;
    for (uint8_t i = 0; i < count_; ++i) {
        if (intr_enable_ && ((0x01 << i) & intr_read_) == 0) {
            continue;
        }

        encoder = encoders_[i];
        pauseInterrupts(i);
        sw = encoder->getSwitch();
        encoder_event_.delta(encoder->getDelta());
        resumeInterrupts(i);

        if (sw != 0) {
            keypress_event_.id(i * 2 + 1);
            keypress_event_.pressed(sw == 1);
        }
        if (encoder_event_.delta() != 0) {
            encoder_event_.id(i * 2);
            yield(encoder_event_);
        }
    }
    intr_read_ = 0;
}

void RotaryEncoderGroup::enableInterrupts(
        void (*pause_cb)(uint8_t), void (*resume_cb)(uint8_t)) {
    intr_enable_ = true;
    intr_read_ = 0;
    pause_intr_cb_ = pause_cb;
    resume_intr_cb_ = resume_cb;
}

void RotaryEncoderGroup::disableInterrupts() {
    intr_enable_ = false;
    intr_read_ = 0;
    pause_intr_cb_ = 0;
    resume_intr_cb_ = 0;
}

void RotaryEncoderGroup::interrupt(uint8_t read) {
    if (read >= 8) {
        intr_read_ = 0xFF;
        return;
    }
    intr_read_ = 0x01 << read;
}

void RotaryEncoderGroup::pauseInterrupts(uint8_t n) {
    if (intr_enable_ && pause_intr_cb_ != nullptr) {
        pause_intr_cb_(n);
    }
}

void RotaryEncoderGroup::resumeInterrupts(uint8_t n) {
    if (intr_enable_ && resume_intr_cb_ != nullptr) {
        resume_intr_cb_(n);
    }
}

}  // namespace R51
