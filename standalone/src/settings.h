#ifndef __R51_SETTINGS_H__
#define __R51_SETTINGS_H__

#include <Arduino.h>
#include <Faker.h>

#include "bus.h"
#include "frame.h"


// Valid frame IDs for settings.
enum SettingsFrameId : uint32_t {
    SETTINGS_FRAME_E = 0x71E,
    SETTINGS_FRAME_F = 0x71F,
};

// Send a sequence of frames for managing settings.
class SettingsSequence {
    public:
        // Create a sequence that communicates over the given frame ID.
        SettingsSequence(SettingsFrameId id, Faker::Clock* clock = Faker::Clock::real()) :
            request_id_((uint32_t)id), clock_(clock), started_(0),
            value_(0xFF), state_(0), sent_(false) {}

        // Trigger the sequence. The next call to receive will broadcast the
        // first frame of the sequence. The sequence expects the next frame to
        // match otherwise it resets.
        bool trigger();

        // Return true if the sequence is ready to send.
        bool ready() const;

        // Fill a frame with the next in the sequence if available. Return true
        // if the frame should be sent or false otherwise.
        bool receive(Frame* frame);

        // Send a response frame back to the sequence. The frame frame matches
        // the expected sequence then the sequence advances to the next state.
        // Otherwise the sequence resets.
        void send(const Frame& frame);

    protected:
        // The sequence's request ID.
        uint32_t requestId() const { return request_id_; }

        // The sequence's current state.
        uint8_t state() const { return state_; }

        // Set the value to send with the state frames that require value.
        void setValue(uint8_t value) { value_ = value; }

        // Return the next state. If the returned state matches the incoming
        // rame then the sequence transitions to the new state and a frame for
        // the state is sent. If this returns the no state transition occurs
        // and no frame is sent.
        virtual uint8_t nextE() = 0;
        virtual uint8_t nextF() = 0;
    private:
        const uint32_t request_id_;
        Faker::Clock* clock_;
        uint32_t started_;
        uint8_t value_;
        uint8_t state_;
        bool sent_;
        uint8_t next();
};

// Sequence to initialize communication with the BCM.
class SettingsInit : public SettingsSequence {
    public:
        SettingsInit(SettingsFrameId id, Faker::Clock* clock = Faker::Clock::real()) :
            SettingsSequence(id, clock) {}
    protected:
        uint8_t nextE() override;
        uint8_t nextF() override;
};

// Sequence used to retrieve settings from the BCM.
class SettingsRetrieve : public SettingsSequence {
    public:
        SettingsRetrieve(SettingsFrameId id, Faker::Clock* clock = Faker::Clock::real()) :
            SettingsSequence(id, clock), state2x_(false) {}
    protected:
        uint8_t nextE() override;
        uint8_t nextF() override;
    private:
        bool state2x_;
};

// Sequence used to update a setting in the BCM.
class SettingsUpdate : public SettingsSequence {
    public:
        SettingsUpdate(SettingsFrameId id, Faker::Clock* clock = Faker::Clock::real()) :
            SettingsSequence(id, clock), update_(0), state2x_(false) {}

        // Set the item to update and its value. 
        void setPayload(uint8_t update, uint8_t value);
    protected:
        uint8_t nextE() override;
        uint8_t nextF() override;
    private:
        uint8_t update_;
        bool state2x_;
};

class SettingsReset : public SettingsSequence {
    public:
        SettingsReset(SettingsFrameId id, Faker::Clock* clock = Faker::Clock::real()) :
            SettingsSequence(id, clock), state2x_(false) {}
    protected:
        uint8_t nextE() override;
        uint8_t nextF() override;
    private:
        bool state2x_;
};

// Communicates with the BCM to retrieve and update body control settings.
// Periodically sends state frames with the current settings. Responds to
// control frames to incrementally change settings.
//
// Frame 0x5700: Settings State Frame
//   Byte 0: Interior & Wipers
//     Bit 0: Auto Interior Illumination; 0 off, 1 on
//     Bit 1: Slide Driver Seat Back on Exit; 0 off, 1 on
//     Bit 2: Speed Sensing Wiper Interval; 0 off, 1 on
//     Bit 4-7: unused
//   Byte 1: Headlights
//     Bits 0-1: Auto Headlights Sensitivity; values 0 - 3
//     Bits 2-3: unused
//     Bits 4-7: Auto Headlights Off Delay; multiplier x15; seconds; 0 off
//   Byte 2: Door Locks
//     Bit 0: Selective Door Unlock; 0 off, 1 on
//     bits 1-3: unused
//     Bits 4-7: Auto Re-Lock Time; minutes; 0 off
//   Byte 3: Remote Key
//     Bit 0: Remote Key Response Horn; 0 off, 1 on
//     Bits 2-3: Remote Key Response Lights; 0 off, 1 unlock, 2 lock, 3 on
//   Bytes 4-7: unused
//
// Frame 0x5701: Settings Control Frame
//   Byte 0: Interior & Wipers
//     Bit 0: Toggle Auto Interior Illumination
//     Bit 1: Toggle Slide Driver Seat Back on Exit
//     Bit 2: Toggle Speed Sensing Wiper Interval
//     Bit 3-7: unused
//   Byte 1: Headlights
//     Bit 0: Auto Headlights Sensitivity+
//     Bit 1: Auto Headlights Sensitivity-
//     Bits 2-3: unused
//     Bit 4: Auto Headlights Off Delay+
//     Bit 5: Auto Headlights Off Delay-
//     Bits 6-7: unused
//   Byte 2: Door Locks
//     Bit 0: Toggle Selective Door Unlock
//     Bit 1-3: unused
//     Bit 4: Auto Re-Lock Time+
//     Bit 5: Auto Re-Lock Time-
//     Bit 6-7: unused
//   Byte 3: Remote Key
//     Bit 0: Toggle Remote Key Response Horn
//     Bit 1: unused
//     Bit 2: Remote Key Response Lights+
//     Bit 3: Remote Key Response Lights-
//     Bit 4-7: unused
//   Bytes 4-6: unused
//   Byte 7: State
//     Bit 0: Request Latest Settings
//     Bit 1-6: unused
//     Bit 7: Reset Settings to Default
class Settings : public Node {
    public:
        Settings(Faker::Clock* clock = Faker::Clock::real());

        // Recieve frames from the node.
        void receive(const Broadcast& broadcast) override;

        // Send a frame to the node.
        void send(const Frame& frame) override;

        // Filter sent frames to this node. Matches 0x71E and 0x72E.
        bool filter(const Frame& frame) const override;

        // Exchange init frames with BCM. 
        bool init();

    private:
        enum RemoteKeyResponseLights : uint8_t {
            LIGHTS_OFF = 0,
            LIGHTS_UNLOCK = 1,
            LIGHTS_LOCK = 2,
            LIGHTS_ON = 3,
        };

        enum AutoHeadlightOffDelay : uint8_t {
            DELAY_0S = 0,
            DELAY_30S = 2,
            DELAY_45S = 3,
            DELAY_60S = 4,
            DELAY_90S = 6,
            DELAY_120S = 8,
            DELAY_150S = 10,
            DELAY_180S = 12,
        };

        enum AutoReLockTime : uint8_t {
            RELOCK_OFF = 0,
            RELOCK_1M = 1,
            RELOCK_5M = 5,
        };

        void handleState(const Frame& frame);
        void handleState05(const byte* data);
        void handleState10(const byte* data);
        void handleState21(const byte* data);
        void handleState22(const byte* data);
        void handleControl(const Frame& frame);

        SettingsInit initE_;
        SettingsRetrieve retrieveE_;
        SettingsUpdate updateE_;
        SettingsReset resetE_;
        bool readyE() const;

        SettingsInit initF_;
        SettingsRetrieve retrieveF_;
        SettingsUpdate updateF_;
        SettingsReset resetF_;
        bool readyF() const;

        Faker::Clock* clock_;
        bool state_changed_;
        uint32_t state_last_broadcast_;
        Frame buffer_;
        Frame state_;
        byte control_state_[8];

        // Manage Auto Interior Illumnation setting state. 
        bool getAutoInteriorIllumination() const;
        void setAutoInteriorIllumination(bool value);

        // Manage Auto Headlight Sensitivity setting state. Valid values from 1 to 4.
        uint8_t getAutoHeadlightSensitivity() const;
        void setAutoHeadlightSensitivity(uint8_t value);

        // Manage Auto Headlight Off Delay setting state.
        AutoHeadlightOffDelay getAutoHeadlightOffDelay() const;
        void setAutoHeadlightOffDelay(AutoHeadlightOffDelay value);

        // Manage Speed Sensing Wiper Interval setting state.
        bool getSpeedSensingWiperInterval() const;
        void setSpeedSensingWiperInterval(bool value);

        // Manage Remote Key Response Horn setting state.
        bool getRemoteKeyResponseHorn() const;
        void setRemoteKeyResponseHorn(bool value);

        // Manage Remote Key Response Lights setting state.
        RemoteKeyResponseLights getRemoteKeyResponseLights() const;
        void setRemoteKeyResponseLights(RemoteKeyResponseLights value);

        // Manage Auto Re-Lock Time setting state.
        AutoReLockTime getAutoReLockTime() const;
        void setAutoReLockTime(AutoReLockTime value);

        // Manage Selective Door Unlock setting state.
        bool getSelectiveDoorUnlock() const;
        void setSelectiveDoorUnlock(bool value);

        // Manage Slide Driver SEat Back on Exit setting state.
        bool getSlideDriverSeatBackOnExit() const;
        void setSlideDriverSeatBackOnExit(bool value);

        // Helpers for triggering sequences from control frames.
        bool toggleAutoInteriorIllumination();
        bool nextAutoHeadlightSensitivity();
        bool prevAutoHeadlightSensitivity();
        bool triggerAutoHeadlightSensitivity(uint8_t value);
        bool nextAutoHeadlightOffDelay();
        bool prevAutoHeadlightOffDelay();
        bool toggleSpeedSensingWiperInterval();
        bool toggleRemoteKeyResponseHorn();
        bool nextRemoteKeyResponseLights();
        bool prevRemoteKeyResponseLights();
        bool triggerRemoteKeyResponseLights(uint8_t value);
        bool nextAutoReLockTime();
        bool prevAutoReLockTime();
        bool toggleSelectiveDoorUnlock();
        bool toggleSlideDriverSeatBackOnExit();
        bool retrieveSettings();
        bool resetSettingsToDefault();
};

#endif  // __R51_SETTINGS_H__
