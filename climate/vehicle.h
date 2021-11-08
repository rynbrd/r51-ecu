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

        // Simulate an "Off" button click.
        void climateClickOff() override;

        // Simulate an "Auto" button click.
        void climateClickAuto() override;

        // Simulate an "A/C" button click.
        void climateClickAc() override;

        // Simulate a "Dual" button click.
        void climateClickDual() override;

        // Simulate a "Recirculate" button click.
        void climateClickRecirculate() override;

        // Simulate a "Mode" button click.
        void climateClickMode() override;

        // Simulate a "Front Defrost" button click.
        void climateClickFrontDefrost() override;

        // Simulate a "Rear Defrost" button click.
        void climateClickRearDefrost() override;

        // Simulate a "Fan Up" button click.
        void climateClickFanSpeedUp() override;

        // Simulate a "Fan Down" button click.
        void climateClickFanSpeedDown() override;

        // Simulate a "Driver Temperature Up" button click.
        void climateClickDriverTempUp() override;

        // Simulate a "Driver Temperature Down" button click.
        void climateClickDriverTempDown() override;

        // Simulate a "Passenger Temperature Up" button click.
        void climateClickPassengerTempUp() override;

        // Simulate a "Passenger Temperature Down" button click.
        void climateClickPassengerTempDown() override;

        // Update state from incoming climate state frames. Accepts frames
        // 0x54A, 0x54B, and 0x625.
        void receive(uint32_t id, uint8_t len, byte* data) override;

        // Push state changes to the CAN bus.
        void push() override;

    private:
        // mode cycle order:
        //   face
        //   face + feet
        //   feet
        //   feet + windshield
        //   windshield
        enum Mode : uint8_t {
            MODE_FACE = 0x04,
            MODE_FACE_FEET = 0x08,
            MODE_FEET = 0x0C,
            MODE_FEET_WINDSHIELD = 0x10,
            MODE_WINDSHIELD = 0x34,
            MODE_AUTO_FACE = 0x84,
            MODE_AUTO_FACE_FEET = 0x88,
            MODE_AUTO_FEET = 0x8C,
        };

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

        // State storage. Tracks state so we can make the UI more responsive.
        bool active_;
        bool auto_;
        bool ac_;
        bool dual_;
        bool recirculate_;
        bool front_defrost_;
        bool rear_defrost_;
        uint8_t mode_;
        uint8_t fan_speed_;
        uint8_t driver_temp_;
        uint8_t passenger_temp_;

        // Return true if the climate control system has exited initialization
        // and is taking commands. Climate control commands are noops until
        // this is true.
        bool climateOnline() const;

        // Toggle a function controlled by a single bit. Return false if
        // climate state can't be modified.
        bool toggleFunction(byte* frame, uint8_t offset, uint8_t bit);

        // Toggle the set temperature bit. Return false if climate state can't
        // be modified.
        bool toggleTemperature();

        // Update state from a 0x54A frame.
        void receive54A(uint8_t len, byte* data);

        // Update state from a 0x54B frame.
        void receive54B(uint8_t len, byte* data);

        // Update state from a 0x625 frame.
        void receive625(uint8_t len, byte* data);

        // Update the dashboard from the stored state.
        void updateDash();
};

#endif  // __R51_VEHICLE_H__
