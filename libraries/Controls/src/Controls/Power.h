#ifndef _R51_CONTROLS_POWER_H_
#define _R51_CONTROLS_POWER_H_

#include <Arduino.h>
#include <Core.h>
#include <Blink.h>
#include "Controls.h"
#include "Screen.h"

namespace R51 {

enum class PDMDevice : uint8_t {
    FRONT_LOCKER = 5,   // A1
    REAR_LOCKER  = 8,   // A2
    AIR_COMP     = 9,   // A3
    LIGHT_BAR    = 1,   // E1
    CHASE_LIGHTS = 6,   // E3
    ROCK_LIGHTS  = 2,   // F1
    SPOT_LIGHTS  = 10,  // F2
};

class PowerControls : public Controls {
    public:
        PowerControls(uint8_t keypad_id, uint8_t pdm_id);

        // Handle keypad, PDM, and illum events.
        void handle(const Message& msg, const Caster::Yield<Message>& yield) override;

    private:
        void handleKey(const KeyState* key, const Caster::Yield<Message>& yield);
        void handlePower(const PowerState* power, const Caster::Yield<Message>& yield);
        void handlePowerState(const ScreenPowerState* even, const Caster::Yield<Message>& yield);
        void sendIndicatorCmd(const Caster::Yield<Message>& yield, uint8_t led,
                PowerMode mode, uint8_t duty_cycle, LEDColor color);
        void sendPowerCmd(const Caster::Yield<Message>& yield, PDMDevice device, PowerCmd cmd);
        void illum(const Caster::Yield<Message>& yield);

        uint8_t keypad_id_;
        uint8_t pdm_id_;
        bool power_;
        bool illum_;

        IndicatorCommand indicator_cmd_;
};

}  // namespace R51

#endif  // _R51_CONTROLS_POWER_H_
