#ifndef __R51_VEHICLE_H__
#define __R51_VEHICLE_H__

#include <stdint.h>
#include "connection.h"
#include "dash.h"
#include "listener.h"
#include "settings.h"


// Base class for sending settings commands and managing their responses.
class SettingsCommand {
    public:
        // Available frames.
        enum Frame : uint32_t {
            FRAME_UNSET = 0,
            FRAME_E = 0x71E,
            FRAME_F = 0x71F,
        };

        // Available operations.
        enum Op : uint8_t {
            // Indicates the command is ready to send.
            OP_READY = 0,

            // Enter and exit settings requests.
            OP_ENTER = 1,
            OP_EXIT = 2,

            // Init requests.
            OP_INIT_00 = 3,
            OP_INIT_20 = 4,
            OP_INIT_40 = 5,
            OP_INIT_60 = 6,

            // Settings requests.
            OP_AUTO_INTERIOR_ILLUM = 7,
            OP_AUTO_HL_SENS = 8,
            OP_AUTO_HL_DELAY = 9,
            OP_SPEED_SENS_WIPER = 10,
            OP_REMOTE_KEY_HORN = 11,
            OP_REMOTE_KEY_LIGHT = 12,
            OP_AUTO_RELOCK_TIME = 13,
            OP_SELECT_DOOR_UNLOCK = 14,
            OP_SLIDE_DRIVER_SEAT = 15,
            OP_GET_STATE_71E_10 = 16,
            OP_GET_STATE_71E_2X = 17,
            OP_GET_STATE_71F_05 = 18,
        };

        virtual ~SettingsCommand() {}

        // Return true if the command is ready to send.
        bool ready();

        // Send the command.
        virtual bool send();

        // Must be called in main loop. Resets the command on time-out in the
        // event that CAN frames are missed or the expected response isn't
        // received.
        void loop();

        // Process a received frame.
        virtual void receive(uint32_t id, uint8_t len, byte* data) = 0;

    protected:
        SettingsCommand(Connection* conn, Frame id = FRAME_UNSET) : 
            id_(id), conn_(conn), op_(OP_READY), last_write_(0) {}

        uint32_t id_;

        Op op() const;

        bool sendRequest(Op op, uint8_t value = 0xFF);

        bool matchResponse(uint32_t id, uint8_t len, byte* data);

        bool matchAndSend(uint32_t id, uint8_t len, byte* data,
                Op match, Op send, uint8_t value = 0xFF);

    private:
        Connection* conn_;
        Op op_;
        uint32_t last_write_;

        bool sendControl(Op op, byte prefix0, byte prefix1, byte prefix2, byte value = 0xFF);
};

// Sends an init command sequence.
class SettingsInit : public SettingsCommand {
    public:
        SettingsInit(Connection* conn, Frame id) : SettingsCommand(conn, id) {}

        void receive(uint32_t id, uint8_t len, byte* data) override;
};

// Request current settings from the BCM.
class SettingsState : public SettingsCommand {
    public:
        SettingsState(Connection* conn, Frame id) : SettingsCommand(conn, id) {}

        void receive(uint32_t id, uint8_t len, byte* data) override;

    private:
        uint8_t state_count_;
};

// Sends a 0x71E or 0x71F setting update command sequence.
class SettingsUpdate : public SettingsCommand {
    public:
        SettingsUpdate(Connection* conn) : SettingsCommand(conn) {}

        bool send(Op setting, uint8_t value);

        void receive(uint32_t id, uint8_t len, byte* data) override;

    private:
        Op setting_;
        uint8_t value_;
        uint8_t state_count_;
};

// Control the vehicle settings. Currently this only sends the initialization
// frames in order to init the BCM and avoid a SEL.
class VehicleSettings : public SettingsController, public Listener {
    public:
        VehicleSettings() : can_(nullptr), dash_(nullptr), auto_interior_illum_(false),
            hl_sens_(HL_SENS_2), hl_delay_(HL_DELAY_45S), speed_wiper_(true), remote_horn_(false),
            remote_lights_(LIGHTS_OFF), relock_time_(RELOCK_1M), selective_unlock_(false),
            slide_seat_(false), init_(false), initE_(nullptr), initF_(nullptr),
            stateE_(nullptr), stateF_(nullptr) {};

        ~VehicleSettings();

        // Connect the controller to a CAN bus.
        void connect(Connection* can, DashSettingsController* dash);

        // Toggle the Auto Interior Illumination setting.
        bool toggleAutoInteriorIllumination() override;

        // Cycle the Auto Headlight Sensitivity setting to the next value.
        bool nextAutoHeadlightSensitivity() override;

        // Cycle the Auto Headlight Sensitivity setting to the previous value.
        bool prevAutoHeadlightSensitivity() override;

        // Cycle the Auto Headlight Off Delay to the next setting.
        bool nextAutoHeadlightOffDelay() override;

        // Cycle the Auto Headlight Off Delay to the previous setting.
        bool prevAutoHeadlightOffDelay() override;

        // Toggle the Speed Sensing Wiper Interval setting.
        bool toggleSpeedSensingWiperInterval() override;

        // Toggle the Remote Key Response Horn setting.
        bool toggleRemoteKeyResponseHorn() override;

        // Cycle the Remote Key Response Lights setting to the next value.
        bool nextRemoteKeyResponseLights() override;

        // Cycle the Remote Key Response Lights setting to the previous value.
        bool prevRemoteKeyResponseLights() override;

        // Cycle the Auto Re-Lock Time setting to the next value.
        bool nextAutoReLockTime() override;

        // Cycle the Auto Re-Lock Time setting to the previous value.
        bool prevAutoReLockTime() override;

        // Toggle the Selective Door Unlock setting.
        bool toggleSelectiveDoorUnlock() override;

        // Toggle the Slide Driver Seat Back On Exit setting.
        bool toggleSlideDriverSeatBackOnExit() override;

        // Retrieve the current settings.
        bool retrieveSettings() override;

        // Reset all settings back to their defaults.
        bool resetSettingsToDefault() override;

        // Process incoming frames.
        void receive(uint32_t id, uint8_t len, byte* data) override;

        // Push state changes to the CAN bus.
        void push() override;
    private:
        // Remote Key Response Lights values. 2 bits long.
        enum RemoteLights : uint8_t {
            LIGHTS_OFF = 0x00,
            LIGHTS_UNLOCK = 0x01,
            LIGHTS_LOCK = 0x02,
            LIGHTS_ON = 0x03,
        };

        // Auto Re-Lock Time values. 2 bits long.
        enum ReLockTime : uint8_t {
            RELOCK_1M = 0x00,
            RELOCK_OFF = 0x01,
            RELOCK_5M = 0x02,
        };

        // Auto Headlight Sensitivity values. 2 bits long.
        enum HeadlightSensitivity : uint8_t {
            HL_SENS_1 = 0x03,
            HL_SENS_2 = 0x00,
            HL_SENS_3 = 0x01,
            HL_SENS_4 = 0x02,
        };

        // Auto Headlight Off Delay values. 3 bits long.
        enum HeadlightOffDelay : uint8_t {
            HL_DELAY_0S = 0x01,
            HL_DELAY_30S = 0x02,
            HL_DELAY_45S = 0x00,
            HL_DELAY_60S = 0x03,
            HL_DELAY_90S = 0x04,
            HL_DELAY_120S = 0x05,
            HL_DELAY_150S = 0x06,
            HL_DELAY_180S = 0x07,
        };

        // The CAN bus to push state change frames to.
        Connection* can_;

        // The dashboard to update.
        DashSettingsController* dash_;

        // Cached settings state.
        bool auto_interior_illum_;
        HeadlightSensitivity hl_sens_;
        HeadlightOffDelay hl_delay_;
        bool speed_wiper_;
        bool remote_horn_;
        RemoteLights remote_lights_;
        ReLockTime relock_time_;
        bool selective_unlock_;
        bool slide_seat_;
        bool init_;

        // Init commands.
        SettingsInit* initE_;
        SettingsInit* initF_;
        SettingsState* stateE_;
        SettingsState* stateF_;
        SettingsUpdate* setter_;

        void receiveState05(byte* data);
        void receiveState10(byte* data);
        void receiveState21(byte* data);
        void receiveState22(byte* data);
};

#endif  // __R51_VEHICLE_H__
