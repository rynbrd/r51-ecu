#include "BlinkKeypad.h"

#include <Canny.h>
#include <Caster.h>
#include <Common.h>
#include "Keypad.h"

namespace R51 {
namespace {

using ::Canny::J1939Message;
using ::Caster::Yield;

uint8_t toBlinkColor(KeypadColor color) {
    switch (color) {
        default:
        case KeypadColor::OFF:
            return 0x00;
        case KeypadColor::WHITE:
            return 0x07;
        case KeypadColor::RED:
            return 0x01;
        case KeypadColor::GREEN:
            return 0x02;
        case KeypadColor::BLUE:
            return 0x03;
        case KeypadColor::CYAN:
            return 0x05;
        case KeypadColor::YELLOW:
            return 0x04;
        case KeypadColor::MAGENTA:
            return 0x06;
        case KeypadColor::AMBER:
            return 0x08;
    }
}

uint8_t toBlinkBrightness(uint8_t brightness) {
    if (brightness % 4 != 0) {
        // round up since we expect a value of 1 to turn on the
        // backlight at the lowest level
        return brightness / 4 + 1;
    }
    return brightness / 4;
}

}  // namespace

BlinkKeypad::BlinkKeypad(uint8_t keypad, uint8_t address, uint8_t key_count) :
        keypress_(KeyPressEvent(keypad)), command_(0xEF00, Canny::NullAddress, address),
        address_(address), key_count_(key_count) {
    command_.resize(8);
    command_.data()[0] = 0x04;
    command_.data()[1] = 0x1B;
}

void BlinkKeypad::handle(const Message& msg, const Caster::Yield<Message>& yield) {
    switch (msg.type()) {
        case Message::EVENT:
            if (msg.event().subsystem == (uint8_t)SubSystem::KEYPAD) {
                switch ((KeypadEvent)msg.event().id) {
                    case KeypadEvent::KEY_LED_CMD:
                        handleKeyLEDCommand((KeyLEDCommand*)&msg.event(), yield);
                        break;
                    case KeypadEvent::KEYPAD_DIM_CMD:
                        handleKeypadDimCommand((KeypadDimCommand*)&msg.event(), yield);
                        break;
                    default:
                        break;
                }
            }
            break;
        case Message::J1939_CLAIM:
            handleJ1939Claim(msg.j1939_claim());
            break;
        case Message::J1939_MESSAGE:
            handleJ1939Message(msg.j1939_message(), yield);
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
    keypress_.id(msg.data()[3] - 1);
    keypress_.pressed(msg.data()[4] == 0x01);
    yield(keypress_);
}

void BlinkKeypad::handleKeyLEDCommand(const KeyLEDCommand* cmd,
        const Yield<Message>& yield) {
    if (command_.source_address() == Canny::NullAddress || cmd->id() >= key_count_) {
        return;
    }

    command_.data()[2] = 0x01;
    command_.data()[3] = cmd->id() + 1;
    command_.data()[4] = toBlinkColor(cmd->color());
    if (cmd->color() == KeypadColor::OFF) {
        command_.data()[5] = 0x00;
    } else {
        command_.data()[5] = 0x01;
    }
    yield(command_);
}

void BlinkKeypad::handleKeypadDimCommand(const KeypadDimCommand* cmd,
        const Yield<Message>& yield) {
    if (command_.source_address() == Canny::NullAddress) {
        return;
    }

    command_.data()[2] = 0x03;
    command_.data()[3] = toBlinkBrightness(cmd->brightness());
    command_.data()[4] = 0xFF;
    command_.data()[5] = 0xFF;
    yield(command_);
}

}  // namespace R51