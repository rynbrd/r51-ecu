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

        void setPinOutput(uint8_t pin, bool on, const Yield<Message>& yield);
        void setPinPWM(uint8_t pin, bool duty_cycle, const Yield<Message>& yield);

        uint8_t pdm_;
        Ticker hb_tick_;
        Canny::J1939Message hb_msg_;
        Canny::J1939Message pin_cmd_;
        Canny::J1939Message pwm_cmd_;
        uint8_t state[2];
};

}  // namespace R51

#endif  // _R51_BLINK_KEYBOX_H_
