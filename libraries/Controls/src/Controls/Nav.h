#ifndef _R51_CONTROLS_NAV_H_
#define _R51_CONTROLS_NAV_H_

#include <Arduino.h>
#include <Caster.h>
#include <Core.h>
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
        void handlePowerState(const ScreenPowerState* even, const Caster::Yield<Message>& yield);
        void handlePageState(const ScreenPageState* event);
        void handleAudioInput(const Event& event, const Caster::Yield<Message>& yield);
        void handleClimateInput(const Event& event, const Caster::Yield<Message>& yield);
        void handleSettingsInput(const Event& event, const Caster::Yield<Message>& yield);
        void handlePowerInput(const Event& event, const Caster::Yield<Message>& yield);
        void setPage(NavPage page);
        void illum(const Caster::Yield<Message>& yield);

        uint8_t keypad_id_;
        bool power_;
        bool illum_;
        LongPressButton power_btn_;
        NavPage page_;
};

}  // namespace R51

#endif  // _R51_CONTROLS_NAV_H_
