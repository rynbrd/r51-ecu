#ifndef _R51_VEHICLE_CLIMATE_EVENTS_H_
#define _R51_VEHICLE_CLIMATE_EVENTS_H_

#include <Arduino.h>
#include <Common.h>
#include <Foundation.h>
#include "Units.h"

namespace R51 {

enum class ClimateEvent : uint8_t {
    SYSTEM_STATE    = 0x01, // State event. Holds climate system state.
    AIRFLOW_STATE   = 0x02, // State event. Holds vent and fan speed state.
    TEMP_STATE      = 0x03, // State event. Holds zone temperature settings.

    TURN_OFF_CMD            = 0x10, // Turn off climate control.
    TOGGLE_AUTO_CMD         = 0x11, // Toggle climate auto mode.
    TOGGLE_AC_CMD           = 0x12, // Toggle A/C compressor request on/off.
    TOGGLE_DUAL_CMD         = 0x13, // Toggle dual zone mode.
    TOGGLE_DEFROST_CMD      = 0x14, // Toggle defrost mode.
    INC_FAN_SPEED_CMD       = 0x15, // Increase fan speed.
    DEC_FAN_SPEED_CMD       = 0x16, // Decrease fan speed.
    TOGGLE_RECIRCULATE_CMD  = 0x17, // Toggle airflow recirculation.
    CYCLE_AIRFLOW_MODE_CMD  = 0x18, // Cycle airflow mode (face, feet, etc).
    INC_DRIVER_TEMP_CMD     = 0x19, // Increase driver zone temperature.
    DEC_DRIVER_TEMP_CMD     = 0x1A, // Decrease driver zone temperature.
    INC_PASSENGER_TEMP_CMD  = 0x1B, // Increase passenger zone temperature.
    DEC_PASSENGER_TEMP_CMD  = 0x1C, // Decrease passenger zone temperature.
};

enum ClimateSystemMode : uint8_t {
    CLIMATE_SYSTEM_OFF = 0,
    CLIMATE_SYSTEM_AUTO = 1,
    CLIMATE_SYSTEM_MANUAL = 2,
    CLIMATE_SYSTEM_DEFROST = 3,
};

// Climate temperature state event.
class ClimateTempState : public Event {
    public:
        ClimateTempState() : Event((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::TEMP_STATE, {0x00, 0x00, 0x00, 0x00}) {}

        EVENT_PROPERTY(uint8_t, driver_temp, data[0], data[0] = value)
        EVENT_PROPERTY(uint8_t, passenger_temp, data[1], data[1] = value)
        EVENT_PROPERTY(uint8_t, outside_temp, data[2], data[2] = value)
        EVENT_PROPERTY(Units, units, (Units)data[3], data[3] = (uint8_t)value)
};

// Climate airflow state event.
class ClimateAirflowState : public Event {
    public:
        ClimateAirflowState() : Event((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::AIRFLOW_STATE, {0x00, 0x00}) {}

        EVENT_PROPERTY(uint8_t, fan_speed, data[0], data[0] = value)
        EVENT_PROPERTY(bool, face,
                getBit(data, 1, 0),
                setBit(data, 1, 0, value))
        EVENT_PROPERTY(bool, feet,
                getBit(data, 1, 1),
                setBit(data, 1, 1, value))
        EVENT_PROPERTY(bool, windshield,
                getBit(data, 1, 2),
                setBit(data, 1, 2, value))
        EVENT_PROPERTY(bool, recirculate,
                getBit(data, 1, 3),
                setBit(data, 1, 3, value))
};

// Climate system state event.
class ClimateSystemState : public Event {
    public:
        ClimateSystemState() : Event((uint8_t)SubSystem::CLIMATE, (uint8_t)ClimateEvent::SYSTEM_STATE, {0x00}) {}

        EVENT_PROPERTY(ClimateSystemMode, mode,
                (ClimateSystemMode)(data[0] & 0x03),
                data[0] = ((data[0] & 0xFC) | (uint8_t)value))
        EVENT_PROPERTY(bool, ac,
                getBit(data, 0, 2),
                setBit(data, 0, 2, value))
        EVENT_PROPERTY(bool, dual,
                getBit(data, 0, 3),
                setBit(data, 0, 3, value))
};

}  // namespace R51

#endif  // _R51_VEHICLE_CLIMATE_EVENTS_H_
