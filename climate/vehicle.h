#ifndef __R51_VEHICLE_H__
#define __R51_VEHICLE_H__

#include <stdint.h>
#include "climate.h"
#include "connection.h"
#include "dash.h"
#include "listener.h"

// Controls the vehicle's climate control system over CAN. On initialization it
// is in the "initialization" state and will send some initialization control
// frames to the A/C Auto Amp to bring it online. The controller will send
// control frames at least every 200ms to ensure the A/C Auto Amp remains
// active.
//
// The system also sends state changes to a dashboard if one is provided on
// connect.
class VehicleClimate : public ClimateController, public Listener {
    public:
        // Create a ClimateControl object in its initialization state.
        VehicleClimate();

        // Connect the controller to a CAN bus.
        void connect(Connection* can, DashController* dash);

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

        // Increase the driver zone temperature by the provided amount.
        void increaseClimateDriverTemp(uint8_t value) override;

        // Decrease the driver zone temperature by the provided amount.
        void decreaseClimateDriverTemp(uint8_t value) override;

        // Set the driver zone temperature in degrees from 60 to 90.
        void setClimateDriverTemp(uint8_t temp) override;

        // Increase the passenger zone temperature by the provided amount.
        void increaseClimatePassengerTemp(uint8_t value) override;

        // Decrease the passenger zone temperature by the provided amount.
        void decreaseClimatePassengerTemp(uint8_t value) override;

        // Set the passenger zone temperature in degrees from 60 to 90.
        void setClimatePassengerTemp(uint8_t temp) override;

        // Update state from incoming climate state frames. Accepts frames
        // 0x54A, 0x54B, and 0x625.
        void receive(uint32_t id, uint8_t len, byte* data) override;

        // Push state changes to the CAN bus.
        void push() override;

    private:
        // The CAN bus to push state change frames to.
        Connection* can_;

        // A dashboard to update.
        DashController* dash_;

        // True if the controller has sent its init packates.
        bool init_complete_;

        // Set to true if the control state has been changed. Control frames
        // will be sent on next push call.
        bool frame54x_changed_;

        // The time the last control frame was sent and how frequently to send
        // them.
        uint32_t last_write_;
        uint8_t keepalive_interval_;
        uint8_t write_count_;

        // Control frames. These are standard (11-bit ID) frames.
        byte frame540_[8];
        byte frame541_[8];

        // Track some climate state to ensure we send commands that make sense.
        bool active_;
        bool dual_;
        uint8_t driver_temp_;
        uint8_t passenger_temp_;

        // Return true if the climate control system has exited initialization
        // and is taking commands. Climate control commands are noops until
        // this is true.
        bool climateOnline() const;

        // Toggle a function controlled by a single bit.
        void toggle(byte* frame, uint8_t offset, uint8_t bit);

        // Toggle the set temperature bit.
        void toggleSetTemperatureBit();

        // Set the driver zone temperature in the control frame.
        void setDriverTempByte(uint8_t temp);

        // Set the passenger zone temperature in the control frame.
        void setPassengerTempByte(uint8_t temp);

        // Update state from a 0x54A frame.
        void receive54A(uint8_t len, byte* data);

        // Update state from a 0x54B frame.
        void receive54B(uint8_t len, byte* data);

        // Update state from a 0x625 frame.
        void receive625(uint8_t len, byte* data);
};

#endif  // __R51_VEHICLE_H__
