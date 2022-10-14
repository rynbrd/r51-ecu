#ifndef _R51_COMMON_POWER_H_
#define _R51_COMMON_POWER_H_

#include <Arduino.h>
#include <Foundation.h>
#include "Event.h"

namespace R51 {

enum class PowerEvent : uint8_t {
    POWER_STATE = 0x01, // Sent on power state change.
    FAULT_STATE = 0x02, // Sent on output fault state change. 
    INPUT_STATE = 0x03, // Sent on input state changes.
    POWER_CMD   = 0x10, // Change power state.
};

enum class PowerMode : uint8_t {
    OFF     = 0,    // Pin is off.
    ON      = 1,    // Pin is on.
    PWM     = 2,    // Pin is in PWM mode.
    FAULT   = 3,    // Pin is in fault state.
};

enum class PowerCmd : uint8_t {
    OFF     = 0,    // Turn pin on.
    ON      = 1,    // Turn pin off.
    TOGGLE  = 2,    // Toggle pin on/off.
    PWM     = 3,    // Enable PWM mode.
    RESET   = 4,    // Reset pin fault.
};

// Transmitted by a PDM to indicate the power state of its connected devices.
class PowerState : public Event {
    public:
        PowerState(uint8_t pdm = 0xFF, uint8_t pin = 0xFF) : Event(
                SubSystem::POWER, (uint8_t)PowerEvent::POWER_STATE,
                {pdm, pin}) {}

        EVENT_PROPERTY(uint8_t, pdm, data[0], data[0] = value);
        EVENT_PROPERTY(uint8_t, pin, data[1], data[1] = value);
        EVENT_PROPERTY(PowerMode, mode, (PowerMode)data[2], data[2] = (uint8_t)value);
        EVENT_PROPERTY(uint8_t, duty_cycle, data[3], data[3] = value);
};

// Transmitted by a PDM to indicate the state of input pins.
class InputState : public Event {
    public:
        InputState(uint8_t pdm = 0xFF, uint8_t pin = 0xFF) : Event(
                SubSystem::POWER, (uint8_t)PowerEvent::INPUT_STATE,
                {pdm, pin}) {}

        EVENT_PROPERTY(uint8_t, pdm, data[0], data[0] = value);
        EVENT_PROPERTY(uint8_t, pin, data[1], data[1] = value);
        EVENT_PROPERTY(bool, state, data[2] != 0, data[2] = (uint8_t)value);
};

// Sent to the PDM to set the output of a device.
class PowerCommand : public Event {
    public:
        PowerCommand(uint8_t pdm = 0xFF, uint8_t pin = 0xFF) : Event(
                SubSystem::POWER, (uint8_t)PowerEvent::POWER_CMD,
                {pdm, pin}) {}

        EVENT_PROPERTY(uint8_t, pdm, data[0], data[0] = value);
        EVENT_PROPERTY(uint8_t, pin, data[1], data[1] = value);
        EVENT_PROPERTY(PowerCmd, cmd, (PowerCmd)data[2], data[2] = (uint8_t)value);
        EVENT_PROPERTY(uint8_t, duty_cycle, data[3], data[3] = value);
};

}  // namespace R51

#endif  // _R51_COMMON_POWER_H_
