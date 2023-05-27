#ifndef _R51_CONTROLS_POWER_H_
#define _R51_CONTROLS_POWER_H_

#include <Arduino.h>
#include <Core.h>
#include <Blink.h>
#include "Controls.h"
#include "Screen.h"

namespace R51 {

// PDM pin mappings:
//  0   B3
//  1   E1
//  2   F1
//  3   C1
//  4   B1
//  5   A1
//  6   E3
//  7   F3
//  8   A2
//  9   A3
//  10  F2
//  11  C2 (PWM)
//  12  D1 (PWM)

enum class PDMDevice : uint8_t {
    LIGHT_BAR        = 5,   // A1
    LEFT_SPOT_LIGHT  = 8,   // A2
    RIGHT_SPOT_LIGHT = 9,   // A3
    RUNNING_LIGHTS   = 11,  // C2
    ROCK_LIGHTS      = 3,   // C1
    CHASE_LIGHTS     = 12,  // D1
    REVERSE_LIGHTS   = 1,   // E1
    EXTRA            = 6,   // E3
    AIR_COMP         = 2,   // F1
    FRONT_LOCKER     = 10,  // F2
    REAR_LOCKER      = 7,   // F3
};

class PowerControls : public Controls {
    public:
        PowerControls(uint8_t keypad_id, uint8_t pdm_id);

        // Handle keypad, PDM, and illum events.
        void handle(const Message& msg, const Caster::Yield<Message>& yield) override;

    private:
        void handleKey(const KeyState* key, const Caster::Yield<Message>& yield);
        void handleIPDM(const Event* event, const Caster::Yield<Message>& yield);
        void handlePower(const PowerState* power, const Caster::Yield<Message>& yield);
        void handlePowerState(const ScreenPowerState* event, const Caster::Yield<Message>& yield);
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
