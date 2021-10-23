#ifndef __CLIMATE_H__
#define __CLIMATE_H__

#include <stdint.h>
#include "mcp_can.h"

// Controls the vehicle's climate control system over CAN. On initialization it
// is in the "intiialization" state and will attempt to handshake with the A/C
// Auto Amp to bring it online. The controller will heartbeat control frames at
// least every 200ms to ensure the A/C Auto Amp remains active.
class ClimateController {
    public:
        // Create a ClimateControl object in its initialization state.
        ClimateController();

        // Return true if the climate control system has exited initialization
        // and is taking commands. Climate control commands are noops until
        // this is true.
        bool online() const;

        // Turn off the climate control.
        void deactivate();

        // Toggle the auto setting.
        void toggleAuto();

        // Toggle the AC setting.
        void toggleAc();

        // Toggle the dual zone setting.
        void toggleDual();

        // Toggle air recirculation.
        void toggleRecirculate();

        // Cycle the airflow mode.
        void cycleMode();

        // Toggle front defrost. This toggles windshield airflow.
        void toggleFrontDefrost();

        // Toggle rear defrost. This toggles the rear window heating element.
        // Currently a noop.
        void toggleRearDefrost();

        // Increase the fan speed by one notch.
        void increaseFanSpeed();

        // Decrease the fan speed by one notch.
        void decreaseFanSpeed();

        // Set the driver zone temperature in degrees from 60 to 90.
        void setDriverTemp(uint8_t temp);

        // Set the passenger zone temperature in degrees from 60 to 90.
        void setPassengerTemp(uint8_t temp);

        // Update state from an incoming 0x54B frame. This frame contains the
        // current A/C Auto Amp state and is needed to perform the
        // initialization handshake.
        void update(uint32_t id, uint8_t len, byte* data);

        // Send pending control frames.
        void emit(MCP_CAN* can);

    private:
        // True if the A/C Auto Amp is operational.
        bool unit_online_;

        // Set to true if the control state has been changed. Control frames
        // will be sent on next emit call.
        bool state_changed_;

        // The time the last control frame was sent and how frequently to send
        // them.
        uint32_t last_heartbeat_;
        uint8_t heartbeat_delay_;

        // Control frames. These are standard (11-bit ID) frames.
        byte frame540_[8];
        byte frame541_[8];
};

#endif
