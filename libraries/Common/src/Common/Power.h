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
    PWM     = 3,    // Pin is in PWM mode.
    FAULT   = 4,    // Pin is in fault state.
};

// Transmitted by a PDM to indicate the power state of its connected devices.
enum class PowerState : public Event {
    public:
        PowerState() : Event(
                SubSystem::POWER, (uint8_t)PowerEvent::POWER_STATE,
                {0xFF, 0x00, 0x00, 0x00, 0x00});

        EVENT_PROPERTY(uint8_t, pdm, data[0], data[0] = value);
        EVENT_PROPERTY(uint8_t, pin, data[1], data[1] = value);
        EVENT_PROPERTY(PowerMode, mode, (Powermode)data[2], data[2] = (uint8_t)value);
        EVENT_PROPERTY(uint8_t, duty_cycle, data[3], data[3] = value);
};

// Transmitted by a PDM to indicate the state of input pins.
enum class InputState : public Event {
    public:
        PowerState() : Event(
                SubSystem::POWER, (uint8_t)PowerEvent::INPUT_STATE,
                {0xFF, 0x00, 0x00, 0x00, 0x00});

        EVENT_PROPERTY(uint8_t, pdm, data[0], data[0] = value);
        EVENT_PROPERTY(uint8_t, pin, data[1], data[1] = value);
        EVENT_PROPERTY(bool, state, data[2] != 0, data[2] = (uint8_t)value);
};

// Sent to the PDM to set the output of a device.
enum class PowerCommand : public Event {
    public:
        PowerCmd(uint8_t n = 0xFF) : Event(
                SubSystem::POWER, (uint8_t)PowerEvent::POWER_CMD, {n, 0xFF, 0x00, 0x00});

        PowerCmd(uint8_t n, bool value) : Event(
                SubSystem::POWER, (uint8_t)PowerEvent::POWER_CMD, {n, value});

        EVENT_PROPERTY(uint8_t, pdm, data[0], data[0] = value);
        EVENT_PROPERTY(uint8_t, pin, data[1], data[1] = value);
        EVENT_PROPERTY(PowerMode, mode, (PowerMode)data[2], data[2] = (uint8_t)value);
        EVENT_PROPERTY(uint8_t, duty_cycle, data[3], data[3] = value);
};

}  // namespace R51

#endif  // _R51_COMMON_POWER_H_
