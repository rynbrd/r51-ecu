#ifndef __R51_REALDASH_H__
#define __R51_REALDASH_H__

#include <Arduino.h>
#include "CRC32.h"
#include "climate.h"
#include "dash.h"
#include "listener.h"
#include "receiver.h"

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
 *   Bytes 5-7: unused
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
 *     Bit 4: driver temp set
 *     Bit 5: passenger temp +
 *     Bit 6: passenger temp -
 *     Bit 7: passenger temp set
 *   Byte 2: Driver Temperature Set
 *   Byte 3: Passenger Temperature Set
 *   Byte 4: Heating Elements
 *     Bit 0: toggle rear window heating element
 *     Bit 1-7: unused
 *   Bytes 5-7: unused
 *
 *   When RealDash connects it always sends an initial control frame with a
 *   zero value. If this system does not receive a control frame after 1000ms
 *   it will assume the RealDash device has disconnected and reset its internal
 *   state to zero. This way when RealDash reconnects and sends a zero control
 *   frame the two systems will be in sync.
 */

// Reads and writes frames to RealDash over serial. Supports RealDash 0x44 and
// 0x66 type frames. All written frames are 0x66 for error checking (0x44
// frames do not contain a checksum).
class RealDashReceiver : public Receiver {
    public:
        // Construct an uninitialized RealDash instance.
        RealDashReceiver();

        // Start the RealDash instance. Data is transmitted over the given
        // serial stream. This is typically Serial or SerialUSB.
        void begin(Stream* stream);

        // Read a frame from RealDash. Returns true if a frame was read or
        // false if not. Should be called on every loop or the connected serial
        // device may block.
        bool read(uint32_t* id, uint8_t* len, byte* data) override;

        // Write frame to RealDash. Return false on success or false on
        // failure.
        bool write(uint32_t id, uint8_t len, byte* data) override;

    private:
        Stream* stream_;

        // Read attributes.
        bool frame_type_66_;        // Type of frame. False if 0x44, true if 0x66.
        uint8_t frame44_checksum_;  // Frame 0x44 checksum.  Calculated as bytes are read.
        CRC32 frame66_checksum_;    // Frame 0x66 checksum. Calculated as bytes are read.
        byte checksum_buffer_[4];   // Buffer to read in the checksum.
        uint8_t frame_size_;        // Expected size of the frame data if type is 0x66.
        uint8_t read_size_;         // Tracks how many bytes have been read.

        // Write attributes.
        CRC32 write_checksum_;

        void updateChecksum(byte b);
        bool readHeader();
        bool readId(uint32_t* id);
        bool readData(uint8_t* len, byte* data);
        bool validateChecksum();
        void reset();
        void writeByte(const byte b);
        void writeBytes(const byte* b, uint8_t len);
};

// Writes frames to RealDash to manage the dashboard state. Frames are repeated
// in order to avoid errors on the line.
class RealDashController : public DashController {
    public:
        // Construct a new controller. Sent frames are repeated repeat times.
        RealDashController(uint8_t repeat = 5) :
            realdash_(nullptr), repeat_(repeat), write_count_(0), last_write_(0) {}

        // Connect the controller to a dashboard and vehicle systems.
        void connect(RealDashReceiver* realdash);

        // Update the on/off state of the climate control.
        void setClimateActive(bool value) override;

        // Update the auto setting of the climate control.
        void setClimateAuto(bool value) override;

        // Update the state of the A/C compressor.
        void setClimateAc(bool value) override;

        // Update dual zone state.
        void setClimateDual(bool value) override;

        // Update state of the face vents.
        void setClimateFace(bool value) override;

        // Update state of the feet vents.
        void setClimateFeet(bool value) override;

        // Update state of air recirculation.
        void setClimateRecirculate(bool value) override;

        // Update state of the front defrost.
        void setClimateFrontDefrost(bool value) override;

        // Update state of the rear defrost.
        void setClimateRearDefrost(bool value) override;

        // Update ste of the fan speed.
        void setClimateFanSpeed(uint8_t value) override;

        // Update the driver temperature state.
        void setClimateDriverTemp(uint8_t value) override;

        // Update the passenger temperature state.
        void setClimatePassengerTemp(uint8_t value) override;

        // Send state changes to RealDash.
        void push() override;

    private:
        RealDashReceiver* realdash_;

        // State frame sent to update RealDash.
        byte frame5400_[5];

        // How many times to repeat a frame sent to RealDash.
        uint8_t repeat_;

        // How many times the current control frame has been written.
        uint8_t write_count_;

        // The last time a control frame was sent.
        uint32_t last_write_;

};

// Process control frames from RealDash.
class RealDashListener : public FrameListener {
    public:
        // Construct an unconnected RealDash listener.
        RealDashListener() : climate_(nullptr), last_receive_(0) {}

        // Connect the RealDash listener to a climate control system.
        void connect(ClimateController* climate);

        // Process frames from RealDash.
        void receive(uint32_t id, uint8_t len, byte* data) override;

    private:
        // The climate control system to send commands to.
        ClimateController* climate_;

        // The timestamp the last control frame was received.
        uint32_t last_receive_;

        // Most recent RealDash 0x5401 climate control payload. Climate control
        // functions are triggered when new frames come in whose bits differ
        // from this value.
        byte frame5401_[8];
};

#endif  // __R51_REALDASH_H__
