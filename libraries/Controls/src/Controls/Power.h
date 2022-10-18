#ifndef _R51_CONTROLS_POWER_H_
#define _R51_CONTROLS_POWER_H_

#include <Arduino.h>
#include <Common.h>
#include <Blink.h>
#include "Controls.h"

namespace R51 {

enum class PDMDevice : uint8_t {
    FRONT_LOCKER = 6,   // A1
    REAR_LOCKER  = 9,   // A2
    AIR_COMP     = 10,  // A3
    LIGHT_BAR    = 2,   // E1
};

class PowerControls : public Controls {
    public:
        PowerControls(uint8_t keypad_id, uint8_t pdm_id);

        // Handle keypad, PDM, and illum events.
        void handle(const Message& msg, const Caster::Yield<Message>& yield) override;

    private:
        void handleKey(const KeyState* key, const Caster::Yield<Message>& yield);
        void handlePower(const PowerState* power, const Caster::Yield<Message>& yield);
        void sendIndicatorCmd(const Caster::Yield<Message>& yield, uint8_t led,
                PowerMode mode, uint8_t duty_cycle, LEDColor color);
        void sendPowerCmd(const Caster::Yield<Message>& yield, PDMDevice device, PowerCmd cmd);

        uint8_t keypad_id_;
        uint8_t pdm_id_;

        IndicatorCommand indicator_cmd_;
};

}  // namespace R51

#endif  // _R51_CONTROLS_POWER_H_
