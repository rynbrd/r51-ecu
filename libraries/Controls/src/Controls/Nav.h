#ifndef _R51_CONTROLS_NAV_H_
#define _R51_CONTROLS_NAV_H_

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>
#include <Faker.h>
#include "Controls.h"
#include "Screen.h"

namespace R51 {

enum class NavPage : uint8_t {
    NONE = 0,
    AUDIO = 1,
    CLIMATE = 2,
    SETTINGS = 3,
};

// Navigation controls for the HMI display.
class NavControls : public Controls {
    public:
        NavControls(uint8_t encoder_keypad_id,
                Faker::Clock* clock = Faker::Clock::real());

        // Handle rotary encoder keypad and illum events.
        void handle(const Message& msg, const Caster::Yield<Message>& yield) override;

        // Emit timed key events.
        void emit(const Caster::Yield<Message>& yield) override;

    private:
        void handlePage(const ScreenPageState* event);
        void handleAudio(const Event& event, const Caster::Yield<Message>& yield);
        void handleClimate(const Event& event, const Caster::Yield<Message>& yield);
        void handleSettings(const Event& event, const Caster::Yield<Message>& yield);
        void setPage(NavPage page);

        uint8_t keypad_id_;
        LongPressButton power_;
        NavPage page_;
};

}  // namespace R51

#endif  // _R51_CONTROLS_NAV_H_
