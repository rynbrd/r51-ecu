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
        hb_tick_(kHeartbeatInterval, false, clock),
        hb_msg_(0xEE00, Canny::NullAddress, address, 0x06),
        pin_cmd_(0xEF00, Canny::NullAddress, address, 0x06),
        pwm_cmd_(0xEF00, Canny::NullAddress, address, 0x06),
        pin_state_(0), pin_fault_(0), power_(pdm_id) {
    hb_msg_.resize(4);
    pin_cmd_.resize(8);
    pin_cmd_.data()[0] = 0x04;
    pin_cmd_.data()[1] = 0x1B;
    pin_cmd_.data()[2] = 0x01;
    pwm_cmd_.resize(8);
    pwm_cmd_.data()[0] = 0x04;
    pwm_cmd_.data()[1] = 0x1B;
    pwm_cmd_.data()[2] = 0x03;
    pwm_cmd_.data()[3] = 0x00;
    pwm_cmd_.data()[4] = 0x00;
}

void BlinkKeybox::handle(const Message& msg, const Yield<Message>& yield) {
    switch (msg.type()) {
        case Message::EVENT:
            if (msg.event()->subsystem == (uint8_t)SubSystem::POWER &&
                    msg.event()->id == (uint8_t)PowerEvent::POWER_CMD) {
                handlePowerCommand((PowerCommand*)msg.event(), yield);
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

void BlinkKeybox::emit(const Yield<Message>& yield) {
    if (hb_tick_.active()) {
        hb_tick_.reset();
        yield(hb_msg_);
    }
}

void BlinkKeybox::handlePowerCommand(const PowerCommand* cmd, const Yield<Message>& yield) {
    if (cmd->pdm() != power_.pdm() || pin_cmd_.source_address() == Canny::NullAddress) {
        return;
    }
    switch (cmd->cmd()) {
        case PowerCmd::ON:
            if (cmd->pin() == 11 || cmd->pin() == 12) {
                setPWM(cmd->pin(), 0xFF, yield);
            } else {
                setOutput(cmd->pin(), true, yield);
            }
            break;
        case PowerCmd::OFF:
            if (cmd->pin() == 11 || cmd->pin() == 12) {
                setPWM(cmd->pin(), 0x00, yield);
            } else {
                setOutput(cmd->pin(), false, yield);
            }
            break;
        case PowerCmd::RESET:
            reset(cmd->pin(), yield);
            break;
        case PowerCmd::TOGGLE:
            {
                if (cmd->pin() == 11) {
                    if (pwm_cmd_.data()[3] == 0x00) {
                        setPWM(cmd->pin(), 0xFF, yield);
                    } else {
                        setPWM(cmd->pin(), 0x00, yield);
                    }
                } else if (cmd->pin() == 12) {
                    if (pwm_cmd_.data()[4] == 0x00) {
                        setPWM(cmd->pin(), 0xFF, yield);
                    } else {
                        setPWM(cmd->pin(), 0x00, yield);
                    }
                } else {
                    bool current = (pin_state_ >> cmd->pin()) & 1;
                    setOutput(cmd->pin(), !current, yield);
                }
            }
            break;
        case PowerCmd::PWM:
            setPWM(cmd->pin(), cmd->duty_cycle(), yield);
            break;
    }
}

void BlinkKeybox::handleJ1939Claim(const J1939Claim& claim) {
    pin_cmd_.source_address(claim.address());
    pwm_cmd_.source_address(claim.address());
    hb_msg_.source_address(claim.address());
    hb_tick_.resume();
}

void BlinkKeybox::handleJ1939Message(const Canny::J1939Message& msg, const Yield<Message>& yield) {
    if (pin_cmd_.source_address() == Canny::NullAddress ||
            msg.source_address() != pin_cmd_.dest_address() ||
            msg.pgn() != 0xEF00 ||
            msg.data()[0] != 0x04 ||
            msg.data()[1] != 0x1B ||
            msg.data()[2] != 0x01) {
        return;
    }

    uint8_t pin = msg.data()[3] - 1;
    bool value = msg.data()[4];
    bool current = (pin_fault_ >> pin) & 1;
    if (current == value) {
        return;
    }
    if (value) {
        pin_fault_ |= (1 << pin);
        if (pin == 11) {
            pwm_cmd_.data()[3] = 0x00;
        } else if (pin == 12) {
            pwm_cmd_.data()[4] = 0x00;
        }
    } else {
        pin_fault_ &= ~(1 << pin);
    }

    power_.pin(pin);
    power_.mode(value ? PowerMode::FAULT : PowerMode::OFF);
    power_.duty_cycle(0xFF);
    yield(power_);
}

void BlinkKeybox::setOutput(uint8_t pin, bool value, const Yield<Message>& yield) {
    bool current = (pin_state_ >> pin) & 1;
    bool fault = (pin_fault_ >> pin) & 1;
    if (current == value || fault) {
        return;
    }

    if (value) {
        pin_state_ |= (1 << pin);
    } else {
        pin_state_ &= ~(1 << pin);
    }

    pin_cmd_.data()[3] = pin + 1;
    pin_cmd_.data()[4] = (uint8_t)value;
    yield(pin_cmd_);

    power_.pin(pin);
    power_.mode(value ? PowerMode::ON : PowerMode::OFF);
    power_.duty_cycle(0xFF);
    yield(power_);
}

void BlinkKeybox::setPWM(uint8_t pin, uint8_t duty_cycle, const Yield<Message>& yield) {
    uint8_t current;
    if (pin == 11) {
        current = pwm_cmd_.data()[3];
        pwm_cmd_.data()[3] = duty_cycle;
    } else if (pin == 12) {
        current = pwm_cmd_.data()[4];
        pwm_cmd_.data()[4] = duty_cycle;
    } else {
        return;
    }
    if (duty_cycle == current) {
        return;
    }
    yield(pwm_cmd_);

    power_.pin(pin);
    power_.mode(PowerMode::PWM);
    power_.duty_cycle(duty_cycle);
    yield(power_);
}

void BlinkKeybox::reset(uint8_t pin, const Yield<Message>& yield) {
    bool current = (pin_fault_ >> pin) & 1;
    if (!current) {
        return;
    }
    pin_fault_ &= ~(1 << pin);

    pin_cmd_.data()[3] = pin + 1;
    pin_cmd_.data()[4] = 0x02;
    yield(pin_cmd_);

    power_.pin(pin);
    power_.mode(PowerMode::OFF);
    power_.duty_cycle(0xFF);
    yield(power_);
}

}  // namespace R51
