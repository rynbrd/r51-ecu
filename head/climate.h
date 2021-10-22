#ifndef __ECU_CLIMATE__
#define __ECU_CLIMATE__

#include "can.h"
#include "status.h"

namespace ECU {

// Manages the climate control system over CAN.
class ClimateControl {
    public:
        // 
        ClimateControl();

        // Return true if the climate control is off.
        bool isOff() const;

        // Turn off the climate control.
        void turnOff();

        // Return true if auto mode is enabled. Returns false when off.
        bool isAuto() const;

        // Toggle the auto setting.
        void toggleAuto();

        // Return true if A/C is enabled. Returns false when off.
        bool isAc() const;

        // Toggle the AC setting.
        void toggleAc();

        // Return true if face air vents are open. Returns false when off.
        bool isFace() const;

        // Return true if feet air vents are open. Returns false when off.
        bool isFeet() const;

        // Return true if windshield (defrost) air vents are open. Returns
        // false when off.
        bool isWindshield() const;

        // Cycle the airflow mode.
        void cycleMode();

        // Toggle front defrost. This toggles windshield airflow.
        void toggleFrontDefrost();

        // Return true if dual zone climate control is active. Returns false
        // when off.
        bool isDualZone() const;

        // Toggle the dual zone setting.
        void toggleDualZone();

        // Return true if recirculation is enabled.
        bool isRecirculating() const;

        // Toggle air recirculation.
        void toggleRecirculation();

        // Return the fan speed. Valid values are between 0 (off) and 8 (max speed).
        uint8_t getFanSpeed() const;

        // Increase the fan speed by one notch.
        void increaseFanSpeed();

        // Decrease the fan speed by one notch.
        void decreaseFanSpeed();

        // Return the driver zone temperature in Fahrenheit. Return value is 0
        // when or from 60 to 90 when active.
        uint8_t getDriverTemp() const;

        // Set the driver zone temperature in degrees from 60 to 90.
        void setDriverTemp(uint8_t temp);

        // Return the passenger zone temperature in Fahrenheit. Return value is
        // 0 when or from 60 to 90 when active.
        uint8_t getPassengerTemp() const;

        // Set the passenger zone temperature in degrees from 60 to 90.
        void setPassengerTemp(uint8_t temp);

        // Process an incoming frame to update the stored state. Accepts frames
        // 54A and 54B.
        void process(const Frame& frame);

        // Emit control frames if any control methods have been called.
        void emit(CanTranceiver* tranceiver);

    private:
        // Set to true if control frames should be sent.
        bool emit_;

        // The time the last control frame was sent.
        uint32_t heartbeat_;

        // Control frames.
        Frame frame540_;
        Frame frame541_;

        // Current state.
        bool running_;
        bool off_;
        bool auto_;
        bool ac_;
        bool face_;
        bool feet_;
        bool windshield_;
        bool dual_;
        bool recirculate_;
        uint8_t fan_speed_;
        uint8_t driver_temp_;
        uint8_t passenger_temp_;

        // Process a 54A frame.
        void process54A(const Frame& frame);

        // Process a 54B frame.
        void process54B(const Frame& frame);

        bool operational() const;
};

}

#endif
