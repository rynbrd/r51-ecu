#include "Keypad.h"

#include <Canny.h>
#include <Caster.h>
#include <Common.h>

namespace R51 {
namespace {

using ::Canny::J1939Message;
using ::Caster::Yield;

uint8_t toBlinkColor(LEDColor color) {
    switch (color) {
        case LEDColor::WHITE:
            return 0x07;
        case LEDColor::RED:
            return 0x01;
        case LEDColor::GREEN:
            return 0x02;
        case LEDColor::BLUE:
            return 0x03;
        case LEDColor::CYAN:
            return 0x05;
        case LEDColor::YELLOW:
            return 0x09;
        case LEDColor::MAGENTA:
            return 0x06;
        case LEDColor::AMBER:
            return 0x08;
        default:
            return 0x00;
    }
}

uint8_t toBlinkBrightness(uint8_t brightness) {
    if (brightness == 0x00) {
        return 0x00;
    } else if (brightness < 0x04) {
        return 0x01;
    }
    return brightness / 4;
}

}  // namespace

BlinkKeypad::BlinkKeypad(uint8_t address, uint8_t keypad, uint8_t key_count) :
        keypress_(KeyState(keypad)), command_(0xEF00, Canny::NullAddress, address, 0x06),
        key_count_(key_count) {
    command_.resize(8);
    command_.data()[0] = 0x04;
    command_.data()[1] = 0x1B;
}

void BlinkKeypad::init(const Caster::Yield<Message>& yield) {
    setBacklightBrightness(yield, 0x00);
    setKeyBrightness(yield, 0xFF);
    for (uint8_t i = 0; i < key_count_; ++i) {
        setKeyLED(yield, i, LEDMode::OFF);
    }
}

void BlinkKeypad::handle(const Message& msg, const Caster::Yield<Message>& yield) {
    switch (msg.type()) {
        case Message::EVENT:
            if (msg.event()->subsystem == (uint8_t)SubSystem::KEYPAD) {
                switch ((KeypadEvent)msg.event()->id) {
                    case KeypadEvent::INDICATOR_CMD:
                        handleIndicatorCommand((IndicatorCommand*)msg.event(), yield);
                        break;
                    case KeypadEvent::BRIGHTNESS_CMD:
                        handleBrightnessCommand((BrightnessCommand*)msg.event(), yield);
                        break;
                    case KeypadEvent::BACKLIGHT_CMD:
                        handleBacklightCommand((BacklightCommand*)msg.event(), yield);
                        break;
                    default:
                        break;
                }
            }
            break;
        case Message::J1939_CLAIM:
            handleJ1939Claim(*msg.j1939_claim());
            break;
        case Message::J1939_MESSAGE:
            handleJ1939Message(*msg.j1939_message(), yield);
            break;
        default:
            break;
    }
}

void BlinkKeypad::handleJ1939Claim(const J1939Claim& claim) {
    command_.source_address(claim.address());
}

void BlinkKeypad::handleJ1939Message(const J1939Message& msg, const Yield<Message>& yield) {
    if (command_.source_address() == Canny::NullAddress ||
            msg.source_address() != command_.dest_address() ||
            msg.pgn() != 0xEF00 ||
            msg.data()[0] != 0x04 ||
            msg.data()[1] != 0x1B ||
            msg.data()[2] != 0x01) {
        return;
    }
    keypress_.key(msg.data()[3] - 1);
    keypress_.pressed(msg.data()[4] == 0x01);
    yield(keypress_);
}

void BlinkKeypad::handleIndicatorCommand(const IndicatorCommand* cmd,
        const Yield<Message>& yield) {
    if (command_.source_address() == Canny::NullAddress ||
            cmd->led() >= key_count_ ||
            cmd->keypad() != keypress_.keypad()) {
        return;
    }
    setKeyLED(yield, cmd->led(), cmd->mode(), cmd->color(), cmd->alt_color());
}

void BlinkKeypad::handleBrightnessCommand(const BrightnessCommand* cmd,
        const Yield<Message>& yield) {
    if (command_.source_address() == Canny::NullAddress ||
            cmd->keypad() != keypress_.keypad()) {
        return;
    }
    setKeyBrightness(yield, cmd->brightness());
}

void BlinkKeypad::handleBacklightCommand(const BacklightCommand* cmd,
        const Yield<Message>& yield) {
    if (command_.source_address() == Canny::NullAddress ||
            cmd->keypad() != keypress_.keypad()) {
        return;
    }
    if ((uint8_t)cmd->color() != 0xFF) {
        setBacklightColor(yield, cmd->color());
    }
    setBacklightBrightness(yield, cmd->brightness());
}

void BlinkKeypad::setKeyLED(const Yield<Message>& yield, uint8_t key, LEDMode mode,
        LEDColor color, LEDColor alt_color) {
    command_.data()[2] = 0x01;
    command_.data()[3] = key + 1;
    switch (mode) {
        case LEDMode::OFF:
            command_.data()[4] = 0x00;
            command_.data()[5] = 0x00;
            command_.data()[6] = 0xFF;
            break;
        case LEDMode::ON:
            command_.data()[4] = toBlinkColor(color);
            command_.data()[5] = 0x01;
            command_.data()[6] = 0xFF;
            break;
        case LEDMode::BLINK:
            command_.data()[4] = toBlinkColor(color);
            command_.data()[5] = 0x02;
            command_.data()[6] = 0xFF;
            break;
        case LEDMode::ALT_BLINK:
            command_.data()[4] = toBlinkColor(color);
            command_.data()[5] = 0x03;
            command_.data()[6] = toBlinkColor(alt_color);
            break;
    }
    yield(command_);
}

void BlinkKeypad::setKeyBrightness(const Yield<Message>& yield, uint8_t brightness) {
    command_.data()[2] = 0x02;
    command_.data()[3] = toBlinkBrightness(brightness);
    command_.data()[4] = 0xFF;
    command_.data()[5] = 0xFF;
    yield(command_);
}

void BlinkKeypad::setBacklightColor(const Yield<Message>& yield, LEDColor color) {
    command_.data()[2] = 0x7D;
    command_.data()[3] = toBlinkColor(color);
    command_.data()[4] = 0xFF;
    command_.data()[5] = 0xFF;
    yield(command_);
}

void BlinkKeypad::setBacklightBrightness(const Yield<Message>& yield, uint8_t brightness) {
    command_.data()[2] = 0x03;
    command_.data()[3] = toBlinkBrightness(brightness);
    command_.data()[4] = 0xFF;
    command_.data()[5] = 0xFF;
    yield(command_);
}

}  // namespace R51
