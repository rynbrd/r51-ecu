#ifndef _R51_BLINK_KEYBOX_H_
#define _R51_BLINK_KEYBOX_H_

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include <Common.h>
#include <Faker.h>
#include <Foundation.h>

namespace R51 {

class BlinkKeybox : public Caster::Node<Message> {
    public:
        BlinkKeybox(uint8_t address, uint8_t pdm_id,
                Faker::Clock* clock = Faker::Clock::real());

        // Handle J1939 state changes from the Keybox and power events from the
        // internal bus.
        void handle(const Message& msg, const Caster::Yield<Message>& yield) override;

        // Emit timed changes to the Keybox.
        void emit(const Caster::Yield<Message>& yield) override;

    private:
        void handlePowerCommand(const PowerCommand* cmd,
                const Caster::Yield<Message>& yield);
        void handleJ1939Claim(const J1939Claim& claim);
        void handleJ1939Message(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);

        void setOutput(uint8_t pin, bool value, const Caster::Yield<Message>& yield);
        void setPWM(uint8_t pin, uint8_t duty_cycle, const Caster::Yield<Message>& yield);
        void reset(uint8_t pin, const Caster::Yield<Message>& yield);

        Ticker hb_tick_;
        Canny::J1939Message hb_msg_;
        Canny::J1939Message pin_cmd_;
        Canny::J1939Message pwm_cmd_;
        uint16_t pin_state_;
        uint16_t pin_fault_;
        PowerState power_;
};

}  // namespace R51

#endif  // _R51_BLINK_KEYBOX_H_
