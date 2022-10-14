#include "Keybox.h"

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include <Common.h>
#include <Faker.h>

namespace R51 {
namespace {

using ::Caster::Yield;

static const uint32_t kHeartbeatInterval = 500;
static const uint32_t kBlinkInterval = 666; // 90 times per minute

}  // namespace

BlinkKeybox::BlinkKeybox(uint8_t address, uint8_t pdm_id, Faker::Clock* clock) :
        pdm_(pdm_id), hb_tick_(kHeartbeatInterval, false, clock),
        hb_msg_(0xEE00, Canny::NullAddress, address, 0x05),
        pin_cmd_(0xEF00, Canny::NullAddress, address, 0x06),
        pwm_cmd_(0xEF00, Canny::NullAddress, address, 0x06) {
    pin_cmd_.resize(8);
    pin_cmd_.data[0] = 0x04;
    pin_cmd_.data[1] = 0x1B;
    pin_cmd_.data[2] = 0x01;
    pwm_cmd_.resize(8);
    pwm_cmd_.data[0] = 0x04;
    pwm_cmd_.data[1] = 0x1B;
    pwm_cmd_.data[2] = 0x03;
    pwm_cmd_.data[3] = 0x00;
    pwm_cmd_.data[4] = 0x00;
}

void KeyBox::handle(const Message& msg, const Yield<Message>& yield) {
    switch (msg.type()) {
        case Message::EVENT:
            if (msg.event().subsystem == (uint8_t)SubSystem::POWER &&
                    msg.event().id == (uint8_t)PowerEvent::POWER_CMD) {
                handlePowerCommand((PowerCommand*)&msg.event(), yield);
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

void BlinkKeybox::emit(const Yield<Message>& yield) {
    if (hb_tick_.active()) {
        hb_tick_.reset();
        yield(hb_msg_);
    }
}

void BlinkKeybox::handleRequestCommand(const RequestCommand* cmd, const Yield<Message>& yield) {
    if (pin_cmd_.source_address() == Canny::NullAddress) {
        return;
    }
}

void BlinkKeybox::handlePowerCommand(const PowerCommand* cmd, const Yield<Message>& yield) {
    if (cmd->pdm() != pdm_id_ || pin_cmd_.source_address() == Canny::NullAddress) {
        return;
    }
    switch (cmd->mode()) {
        case PowerMode::ON:
            if (pin == 11 || pin == 12) {
                setPinPWM(cmd->device(), 0xFF, yield);
            } else {
                setPinOutput(cmd->device(), 0x01, yield);
            }
            break;
        case PowerMode::OFF:
            if (pin == 11 || pin == 12) {
                setPinPWM(cmd->device(), 0x00, yield);
            } else {
                setPinOutput(cmd->device(), 0x00, yield);
            }
            break;
        case PowerMode::PWM:
            setPinPWM(cmd->device(), cmd->duty_cycle(), yield);
            break;
    }
}

void BlinkKeybox::handleJ1939Claim(const J1939Claim& claim) {
    command_.source_address(claim.address());
    hb_msg_.source_address(claim.address());
    hb_tick_.resume();
}

void BlinkKeybox::handleJ1939Message(const Canny::J1939Message& msg, const Yield<Message>& yield) {
    if (command_.source_address() == Canny::NullAddress) {
        return;
    }
}

void BlinkKeybox::setPinOutput(uint8_t pin, uint8_t mode, const Yield<Message>& yield) {
    if (pin > 12) {
        return;
    }
    pin_cmd_.data()[3] = pin - 1;
    pin_cmd_.data()[4] = mode;
    yield(pin_cmd_);
}

void BlinkKeybox::setPinPWM(uint8_t pin, uint8_t duty_cycle, const Yield<Message>& yield) {
    if (pin == 11) {
        pwm_cmd_.data()[3] == duty_cycle;
    } else if (pin == 12) {
        pwm_cmd_.data()[4] == duty_cycle;
    } else {
        return;
    }
    yield(pwm_cmd_);
}

}  // namespace R51
