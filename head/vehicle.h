#ifndef __VEHICLE_H__
#define __VEHICLE_H__

#include <stdint.h>
#include "dash.h"
#include "climate.h"
#include "mcp_can.h"

// Controls the vehicle's climate control system over CAN. On initialization it
// is in the "intiialization" state and will attempt to handshake with the A/C
// Auto Amp to bring it online. The controller will heartbeat control frames at
// least every 200ms to ensure the A/C Auto Amp remains active.
class VehicleController : public ClimateController {
    public:
        // Create a ClimateControl object in its initialization state.
        VehicleController();

        // Connect the controller to a CAN bus.
        void connect(MCP_CAN* can);

        // Turn off the climate control.
        void deactivateClimate() override;

        // Toggle the auto setting.
        void toggleClimateAuto() override;

        // Toggle the AC setting.
        void toggleClimateAc() override;

        // Toggle the dual zone setting.
        void toggleClimateDual() override;

        // Toggle air recirculation.
        void toggleClimateRecirculate() override;

        // Cycle the airflow mode.
        void cycleClimateMode() override;

        // Toggle front defrost. This toggles windshield airflow.
        void toggleClimateFrontDefrost() override;

        // Toggle rear defrost. This toggles the rear window heating element.
        // Currently a noop.
        void toggleClimateRearDefrost() override;

        // Increase the fan speed by one notch.
        void increaseClimateFanSpeed() override;

        // Decrease the fan speed by one notch.
        void decreaseClimateFanSpeed() override;

        // Set the driver zone temperature in degrees from 60 to 90.
        void setClimateDriverTemp(uint8_t temp) override;

        // Set the passenger zone temperature in degrees from 60 to 90.
        void setClimatePassengerTemp(uint8_t temp) override;

        // Update state from an incoming 0x54B frame. This frame contains the
        // current A/C Auto Amp state and is needed to perform the
        // initialization handshake.
        void update(uint32_t id, uint8_t len, byte* data);

        // Push state changes to the CAN bus.
        void push();

    private:
        // The CAN bus to push state change frames to.
        MCP_CAN* can_;

        // True if the A/C Auto Amp is operational.
        bool climate_online_;

        // Set to true if the control state has been changed. Control frames
        // will be sent on next push call.
        bool frame54x_changed_;

        // The time the last control frame was sent and how frequently to send
        // them.
        uint32_t last_heartbeat_;
        uint8_t heartbeat_delay_;

        // Control frames. These are standard (11-bit ID) frames.
        byte frame540_[8];
        byte frame541_[8];

        // Return true if the climate control system has exited initialization
        // and is taking commands. Climate control commands are noops until
        // this is true.
        bool climateOnline() const;

        // Toggle a function controlled by a single bit.
        bool toggle(byte* frame, uint8_t offset, uint8_t bit);
};

// Listens for vehicle signals and updates state in connected dashboards.
class VehicleListener {
    public:
        // Construct an empty listener.
        VehicleListener() : dash_(nullptr) {}

        // Connect the listener to a dashboard.
        void connect(DashController* controller);

        // Update state from incoming climate state frames. Accepts frames
        // 0x54A, 0x54B, and 0x625.
        void update(uint32_t id, uint8_t len, byte* data);

    private:
        // A dashboard to update.
        DashController* dash_;

        // Update state from a 0x54A frame.
        void update54A(byte* data);

        // Update state from a 0x54B frame.
        void update54B(byte* data);

        // Update state from a 0x625 frame.
        void update625(byte* data);
};

#endif
