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

void RotaryEncoder::setColor(KeypadColor color) {
    switch (color) {
        case KeypadColor::WHITE:
            color_ = 0xFFFFFF;
            break;
        case KeypadColor::RED:
            color_ = 0xFF0000;
            break;
        case KeypadColor::GREEN:
            color_ = 0x00FF00;
            break;
        case KeypadColor::BLUE:
            color_ = 0x0000FF;
            break;
        case KeypadColor::CYAN:
            color_ = 0x00FFFF;
            break;
        case KeypadColor::YELLOW:
            color_ = 0xFFFF00;
            break;
        case KeypadColor::MAGENTA:
            color_ = 0xFF00FF;
            break;
        case KeypadColor::AMBER:
            color_ = 0xFFBF00;
            break;
        default:
            color_ = 0x000000;
            break;
    }
}

void RotaryEncoder::setBrightness(uint8_t value) {
    brightness_ = value;
}

void RotaryEncoder::showPixel() {
    Serial.print("set pixel brightness ");Serial.println(brightness_, HEX);
    Serial.print("set pixel color ");Serial.println(color_, HEX);
    if (brightness_ == 0) {
        // a brightness of 0 is 100% according to Adafruit
        neopixel_.setPixelColor(0, neopixel_.Color(0x00, 0x00, 0x00));
    } else {
        neopixel_.setBrightness(brightness_);
        neopixel_.setPixelColor(0, neopixel_.Color(
                    (color_ & 0xFF0000) >> 16,
                    (color_ & 0x00FF00) >> 8,
                    (color_ & 0x0000FF)));
    }
    neopixel_.show();
}

RotaryEncoderGroup::RotaryEncoderGroup(uint8_t keypad, RotaryEncoder** encoders, uint8_t count) :
        encoders_(encoders), count_(count),
        intr_enable_(false), intr_read_(0),
        pause_intr_cb_(nullptr), resume_intr_cb_(nullptr) {
    encoder_event_.keypad(keypad);
    keypress_event_.keypad(keypad);
}

void RotaryEncoderGroup::handle(const Message& msg, const Yield<Message>&) {
    if (msg.type() != Message::EVENT ||
            msg.event().subsystem != (uint8_t)SubSystem::KEYPAD) {
        return;
    }
    switch ((KeypadEvent)msg.event().id) {
        case KeypadEvent::KEY_LED_CMD:
            handleKeyLEDCommand((KeyLEDCommand*)&msg.event());
            break;
        case KeypadEvent::KEYPAD_DIM_CMD:
            handleKeypadDimCommand((KeypadDimCommand*)&msg.event());
            break;
        default:
            break;
    }
}

void RotaryEncoderGroup::handleKeyLEDCommand(const KeyLEDCommand* cmd) {
    uint8_t n = cmd->id() / 2;
    if (cmd->keypad() != keypress_event_.keypad() || n >= count_) {
        return;
    }
    RotaryEncoder* encoder = encoders_[n];
    pauseInterrupts(n);
    encoder->setColor(cmd->color());
    encoder->showPixel();
    resumeInterrupts(n);
}

void RotaryEncoderGroup::handleKeypadDimCommand(const KeypadDimCommand* cmd) {
    if (cmd->keypad() != keypress_event_.keypad()) {
        return;
    }
    for (uint8_t i = 0; i < count_; ++i) {
        pauseInterrupts(i);
        encoders_[i]->setBrightness(cmd->brightness());
        encoders_[i]->showPixel();
        resumeInterrupts(i);
    }
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
            yield(keypress_event_);
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
