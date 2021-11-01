#ifndef __R51_REALDASH_H__
#define __R51_REALDASH_H__

#include <Arduino.h>
#include "CRC32.h"
#include "climate.h"
#include "dash.h"
#include "listener.h"

/* RealDash Control Frames
 *
 * Frame 0x5400: Climate Controls
 *   Byte 0: Unit Status
 *     Bit 0: active; send 0 to deactivate, 1 to enable auto mode
 *     Bit 1: auto
 *     Bit 2: ac
 *     Bit 3: dual
 *     Bit 4: mode; command only; flip to trigger
 *     Bit 5: face; status only
 *     Bit 6: feet; status only
 *     Bit 7: recirculate
 *   Byte 1: defrost
 *     Bit 0: defrost front
 *     Bit 1: defrost rear
 *     Bit 2: defrost mirrors
 *     Bit 3-7: unused
 *   Byte 2: fan speed
 *     Bits 0-3: fan speed; status only
 *     Bit 4: fan+; command only; flip to trigger
 *     Bit 5: fan-; command only; flip to trigger
 *     Buts 6-7: unused
 *   Byte 3: driver temp
 *     Bits 0-8: driver temp
 *   Byte 4: passenger temp
 *     Bits 0-8: passenger temp
 */

// Reads and writes frames to RealDash over serial. Supports RealDash 0x44 and
// 0x66 type frames. All written frames are 0x66 for error checking (0x44
// frames do not contain a checksum).
class RealDashReceiver {
    public:
        // Construct an uninitialized RealDash instance.
        RealDashReceiver();

        // Start the RealDash instance. Data is transmitted over the given
        // serial stream. This is typically Serial or SerialUSB.
        void begin(Stream* stream);

        // Read a frame from RealDash. Returns true if a frame was read or
        // false if not. Should be called on every loop or the connected serial
        // device may block.
        bool read(uint32_t* id, uint8_t* len, byte* data);

        // Write frame to RealDash. Return false on success or false on
        // failure.
        bool write(uint32_t id, uint8_t len, byte* data);

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

// Manages the RealDash dashboard. A single control frame (0x5400) is exchanged
// with the dashboard to manage the state of the climate control system. The
// frame is sent at least every 200ms to ensure the dashboard remains active
// and has the latest state.
class RealDash : public DashController, public FrameListener  {
    public:
        RealDash(uint8_t repeat);

        // Connect the controller to a dashboard and vehicle systems.
        void connect(RealDashReceiver* realdash, ClimateController* climate);

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

        // Update state of the mirror defrost.
        void setClimateMirrorDefrost(bool value) override;

        // Update ste of the fan speed.
        void setClimateFanSpeed(uint8_t value) override;

        // Update the driver temperature state.
        void setClimateDriverTemp(uint8_t value) override;

        // Update the passenger temperature state.
        void setClimatePassengerTemp(uint8_t value) override;

        // Process a RealDash frame.
        void receive(uint32_t id, uint8_t len, byte* data) override;

        // Send state changes to RealDash.
        void push() override;

    private:
        RealDashReceiver* realdash_;
        ClimateController* climate_; 

        // State frame. Dashboard state is tracked in order to avoid toggling
        // items on/off when not needed.
        byte frame5400_[8];

        // How many times to repeat a frame sent to RealDash.
        uint8_t repeat_;

        // How many times the current control frame has been written.
        uint8_t write_count_;

        // The last time a control frame was sent.
        uint32_t last_write_;

};

#endif  // __R51_REALDASH_H__
