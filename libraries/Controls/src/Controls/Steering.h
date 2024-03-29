#ifndef _R51_CONTROLS_STEERING_H_
#define _R51_CONTROLS_STEERING_H_

#include <Arduino.h>
#include <Caster.h>
#include <Core.h>
#include <Faker.h>
#include "Audio.h"
#include "Controls.h"

namespace R51 {

class SteeringControls : public Controls {
    public:
        SteeringControls(uint8_t steering_keypad_id,
                Faker::Clock* clock = Faker::Clock::real());

        // Handle steering keypad events.
        void handle(const Message& msg, const Caster::Yield<Message>& yield) override;

        // Emit timed key events.
        void emit(const Caster::Yield<Message>& yield) override;

    private:
        uint8_t keypad_id_;
        LongPressButton power_;
        RepeatButton seek_up_;
        RepeatButton seek_down_;
        RepeatButton volume_up_;
        RepeatButton volume_down_;
};

}  // namespace R51

#endif  // _R51_CONTROLS_STEERING_H_
