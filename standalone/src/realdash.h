#ifndef __R51_REALDASH_H__
#define __R51_REALDASH_H__

#include <Arduino.h>
#include <Canny.h>

#include "CRC32.h"
#include "bus.h"


// Reads and writes frames to RealDash over serial. Supports RealDash 0x44 and
// 0x66 type frames. All written frames are 0x66 for error checking (0x44
// frames do not contain a real checksum). This class is abstract. A child
// class needs to implement the filter() method to be complete.
class RealDash : public Node {
    public:
        // Construct an uninitialized RealDash instance.
        RealDash();

        // Start the RealDash instance. Data is transmitted over the given
        // serial stream. This is typically Serial or SerialUSB.
        void begin(Stream* stream);

        // Write frame to RealDash. Return false on success or false on
        // failure.
        void handle(const Canny::Frame& frame) override;

        // Read a frame from RealDash. Returns true if a frame was read or
        // false if not. Should be called on every loop or the connected serial
        // device may block.
        void emit(const Yield& yield) override;

        // Only read frames which match the filter.
        virtual bool readFilter(const Canny::Frame& frame) const = 0;

        // Only write frames which match the filter.
        virtual bool writeFilter(const Canny::Frame& frame) const = 0; 

    private:
        Stream* stream_;
        Canny::Frame frame_;

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
        void writeBytes(uint32_t data);
};

#endif  // __R51_REALDASH_H__
