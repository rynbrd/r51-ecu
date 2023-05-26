#ifndef _R51_CONTROLS_POWER_H_
#define _R51_CONTROLS_POWER_H_

#include <Arduino.h>
#include <Core.h>
#include <Blink.h>
#include "Controls.h"
#include "Screen.h"

namespace R51 {

// PDM pin mappings:
//  1   B3
//  2   E1
//  3   F1
//  4   C1
//  5   B1
//  6   A1
//  7   E3
//  8   F3
//  9   A2
//  10  A3
//  11  F2
//  12  C2 (PWM)
//  13  D1 (PWM)

enum class PDMDevice : uint8_t {
    LIGHT_BAR        = 6,   // A1
    LEFT_SPOT_LIGHT  = 9,   // A2
    RIGHT_SPOT_LIGHT = 10,  // A3
    RUNNING_LIGHTS   = 12,  // C2
    ROCK_LIGHTS      = 4,   // C1
    CHASE_LIGHTS     = 13,  // D1
    REVERSE_LIGHTS   = 2,   // E1
    EXTRA            = 7,   // E3
    AIR_COMP         = 3,   // F1
    FRONT_LOCKER     = 11,  // F2
    REAR_LOCKER      = 8,   // F3
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
