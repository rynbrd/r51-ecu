#ifndef __R51_REALDASH_H__
#define __R51_REALDASH_H__

#include <Arduino.h>
#include "CRC32.h"
#include "bus.h"
#include "connection.h"
#include "dash.h"
#include "listener.h"
#include "settings.h"

/* RealDash Frames
 *
 * Frame 0x5400: Climate State Frame
 *   Byte 0: Unit and Vent State
 *     Bit 0: active
 *     Bit 1: auto
 *     Bit 2: ac
 *     Bit 3: dual
 *     Bit 4: face
 *     Bit 5: feet
 *     Bit 6: defrost (windshield)
 *     Bit 7: recirculate
 *   Byte 1: Fan Speed
 *   Byte 2: Driver Temperature
 *   Byte 3: Passenger Temperature
 *   Byte 4: Heating Elements
 *     Bit 0: rear window heating element
 *     Bit 1-7: unused
 *   Bytes 5-6: unused
 *   Byte 7: Outside Temperature
 *
 * Frame 0x5401: Climate Control Frame
 *   Byte 0: Unit State and Mode
 *     Bit 0: turn off
 *     Bit 1: toggle auto
 *     Bit 2: toggle ac
 *     Bit 3: toggle dual
 *     Bit 4: cycle mode
 *     Bit 5: unused
 *     Bit 6: toggle defrost (windshield)
 *     Bit 7: toggle recirculate
 *   Byte 1: Fan and Temperature Control
 *     Bit 0: fan speed +
 *     Bit 1: fan speed -
 *     Bit 2: driver temp + 
 *     Bit 3: driver temp -
 *     Bit 4: passenger temp +
 *     Bit 5: passenger temp -
 *     Bit 6-7: unused
 *   Byte 2: Driver Temperature Set
 *   Byte 3: Passenger Temperature Set
 *   Byte 4: Heating Elements
 *     Bit 0: toggle rear window heating element
 *     Bit 1-7: unused
 *   Bytes 5-7: unused
 *
 * Frame 0x5700: Settings State Frame
 *   Byte 0: Interior & Wipers
 *     Bit 0: Auto Interior Illumination; 0 off, 1 on
 *     Bit 1: Slide Driver Seat Back on Exit; 0 off, 1 on
 *     Bit 2: Speed Sensing Wiper Interval; 0 off, 1 on
 *     Bit 4-7: unused
 *   Byte 1: Headlights
 *     Bits 0-1: Auto Headlights Sensitivity; values 0 - 3
 *     Bits 2-3: unused
 *     Bits 4-7: Auto Headlights Off Delay; multiplier x15; seconds; 0 off
 *   Byte 2: Door Locks
 *     Bit 0: Selective Door Unlock; 0 off, 1 on
 *     bits 1-3: unused
 *     Bits 4-7: Auto Re-Lock Time; minutes; 0 off
 *   Byte 3: Remote Key
 *     Bit 0: Remote Key Response Horn; 0 off, 1 on
 *     Bits 2-3: Remote Key Response Lights; 0 off, 1 unlock, 2 lock, 3 on
 *   Bytes 4-7: unused
 *
 * Frame 0x5701: Settings Control Frame
 *   Byte 0: Interior & Wipers
 *     Bit 0: Toggle Auto Interior Illumination
 *     Bit 1: Toggle Slide Driver Seat Back on Exit
 *     Bit 2: Toggle Speed Sensing Wiper Interval
 *     Bit 3-7: unused
 *   Byte 1: Headlights
 *     Bit 0: Auto Headlights Sensitivity+
 *     Bit 1: Auto Headlights Sensitivity-
 *     Bits 2-3: unused
 *     Bit 4: Auto Headlights Off Delay+
 *     Bit 5: Auto Headlights Off Delay-
 *     Bits 6-7: unused
 *   Byte 2: Door Locks
 *     Bit 0: Toggle Selective Door Unlock
 *     Bit 1-3: unused
 *     Bit 4: Auto Re-Lock Time+
 *     Bit 5: Auto Re-Lock Time-
 *     Bit 6-7: unused
 *   Byte 3: Remote Key
 *     Bit 0: Toggle Remote Key Response Horn
 *     Bit 1: unused
 *     Bit 2: Remote Key Response Lights+
 *     Bit 3: Remote Key Response Lights-
 *     Bit 4-7: unused
 *   Bytes 4-6: unused
 *   Byte 7: State
 *     Bit 0: Request Latest Settings
 *     Bit 1-6: unused
 *     Bit 7: Reset Settings to Default
 *
 *   When RealDash connects it always sends an initial control frame with a
 *   zero value. If this system does not receive a control frame after the
 *   configured timeout it will assume the RealDash device has disconnected and
 *   reset its internal state to zero. This way when RealDash reconnects and
 *   sends a zero control frame the two systems will be in sync.
 */

// Reads and writes frames to RealDash over serial. Supports RealDash 0x44 and
// 0x66 type frames. All written frames are 0x66 for error checking (0x44
// frames do not contain a checksum). This class is abstract. A child class
// needs to implement the filter() method to be complete.
class RealDash : public Node {
    public:
        // Construct an uninitialized RealDash instance.
        RealDash();

        // Start the RealDash instance. Data is transmitted over the given
        // serial stream. This is typically Serial or SerialUSB.
        void begin(Stream* stream);

        // Read a frame from RealDash. Returns true if a frame was read or
        // false if not. Should be called on every loop or the connected serial
        // device may block.
        void receive(const Broadcast& broadcast) override;

        // Write frame to RealDash. Return false on success or false on
        // failure.
        void send(const Frame& frame) override;

    private:
        Stream* stream_;
        Frame frame_;

        // Read attributes.
        bool frame_type_66_;        // Type of frame. False if 0x44, true if 0x66.
        uint8_t frame44_checksum_;  // Frame 0x44 checksum. Calculated as bytes are read.
        CRC32 frame66_checksum_;    // Frame 0x66 checksum. Calculated as bytes are read.
        byte checksum_buffer_[4];   // Buffer to read in the checksum.
        uint8_t frame_size_;        // Expected size of the frame data if type is 0x66.
        uint8_t read_size_;         // Tracks how many bytes have been read.

        // Write attributes.
        CRC32 write_checksum_;

        void updateChecksum(byte b);
        bool readHeader();
        bool readId();
        bool readData();
        bool validateChecksum();
        void reset();
        void writeByte(const byte b);
        void writeBytes(const byte* b, uint8_t len);
};

class RealDashSettings : public DashSettingsController, Listener {
    public:
        // Construct a new controller. Sent frames are repeated repeat times.
        RealDashSettings(uint8_t repeat = 5) :
            realdash_(nullptr), settings_(nullptr), repeat_(repeat),
            write_count_(0), last_read_(0){}

        // Connect the controller to a dashboard and vehicle systems.
        void connect(Connection* realdash, SettingsController* settings);

        // Update Auto Interior Illumination setting. True is on, false is off.
        void setAutoInteriorIllumination(bool value) override;

        // Update the Auto Headlight Sensitivity setting.
        void setAutoHeadlightSensitivity(uint8_t value) override;

        // Update the Auto Headlight Off Delay setting. Value is in in seconds.
        void setAutoHeadlightOffDelay(DashSettingsController::AutoHeadlightOffDelay value) override;

        // Update the Speed Sensing Wiper Interval setting. True is on, false
        // is off.
        void setSpeedSensingWiperInterval(bool value) override;

        // Update the Remote Key Response Horn setting. True is on, false is
        // off.
        void setRemoteKeyResponseHorn(bool value) override;

        // Update the Remote Key Response Lights setting.
        void setRemoteKeyResponseLights(DashSettingsController::RemoteKeyResponseLights value) override;

        // Update the Auto Re-Lock Time setting. Value is in seconds.
        void setAutoReLockTime(DashSettingsController::AutoReLockTime value) override;

        // Update the Selective Door Unlock setting. True is on, false is off.
        void setSelectiveDoorUnlock(bool value) override;

        // Update the Slide Driver Seat Back On Exit setting. True is on, false is off.
        void setSlideDriverSeatBackOnExit(bool value) override;

        // Process frames from RealDash.
        void receive(uint32_t id, uint8_t len, byte* data) override;

        // Send state changes to RealDash.
        void push() override;
    private:
        // The connection to the RealDash instance.
        Connection* realdash_;

        // The climate control system to send commands to.
        SettingsController* settings_;

        // State frame sent to update RealDash.
        byte frame5700_[8];

        // Most recent settings control frame.
        byte frame5701_[8];

        // How many times to repeat a frame sent to RealDash.
        uint8_t repeat_;

        // How many times the current control frame has been written.
        uint8_t write_count_;

        // The last time a control frame was received.
        uint32_t last_read_;
};

#endif  // __R51_REALDASH_H__