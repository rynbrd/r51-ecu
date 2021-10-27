#ifndef __R51_REALDASH_H__
#define __R51_REALDASH_H__

#include <Arduino.h>
#include "climate.h"
#include "dash.h"
#include "listener.h"

// Reads and writes frames to RealDash over serial. Only supports RealDash 0x44
// type frames. Other incoming frame types are discarded.
class RealDashSerial {
    public:
        // Construct an uninitialized RealDash instance.
        RealDashSerial() : stream_(nullptr) {}

        // Start the RealDash instance. Data is transmitted over the given
        // serial stream. This is typically Serial or SerialUSB.
        void begin(Stream* stream);

        // Read a frame from RealDash. Returns true if a frame was read or
        // false if not. Should be called on every loop or the connected serial
        // device may block.
        bool read(uint32_t* id, uint8_t* len, byte* data);

        // Write frame to RealDash.
        void write(uint32_t id, uint8_t len, byte* data);

    private:
        Stream* stream_;

        // Incoming frame buffer.
        byte incoming_buffer_[17];

        // How many bytes have been written into the incoming buffer. This will
        // be 17 when the buffer is full and a frame is available for read.
        int8_t incoming_size_;
};

// Sends climate state changes to the dashboard.
//
// Frame format:
// id: 5400
//   data:
//     byte 1: unit status, airflow
//       0 active
//       1 auto
//       2 ac
//       3 dual
//       4 face
//       5 feet
//       6 recirculate
//       7 front defrost
//     byte 2: other
//       0 rear defrost
//     byte 3: fan speed
//       0-8 fan speed, 0 to 8
//     byte 5: driver temperature
//       0-8 temperature
//     byte 6: passenger temperature
//       0-8 temperature
class RealDashController : public DashController {
    public:
        RealDashController();

        // Connect the controller to a dashboard.
        void connect(RealDashSerial* realdash);

        // Update the on/off state of the climate control.
        void setClimateActive(bool value);

        // Update the auto setting of the climate control.
        void setClimateAuto(bool value);

        // Update the state of the A/C compressor.
        void setClimateAc(bool value);

        // Update dual zone state.
        void setClimateDual(bool value);

        // Update state of the face vents.
        void setClimateFace(bool value);

        // Update state of the feet vents.
        void setClimateFeet(bool value);

        // Update state of air recirculation.
        void setClimateRecirculate(bool value);

        // Update state of the front defrost.
        void setClimateFrontDefrost(bool value);

        // Update state of the rear defrost.
        void setClimateRearDefrost(bool value);

        // Update ste of the fan speed.
        void setClimateFanSpeed(uint8_t value);

        // Update the driver temperature state.
        void setClimateDriverTemp(uint8_t value);

        // Update the passenger temperature state.
        void setClimatePassengerTemp(uint8_t value);

        // Send state changes to the dashboard.
        void push();

    private:
        RealDashSerial* realdash_;
        byte frame5400_[8];
        bool frame5400_changed_;
};

// Process RealDash frames into vehicle control commands.
//
// Frame 5401: Set Climate
//   Byte 1: unit status, airflow
//     0 turn off
//     1 toggle auto
//     2 toggle a/c 
//     3 toggle dual zone
//     4 toggle recirculate
//     5 cyle airflow mode
//     6 toggle front defrost
//     7 toggle rear defrost
//     8 always 0
//   Byte 2: fan speed
//     0 increase fan speed
//     1 decrease fan speed
//     2-8 always 0
//   Byte 3: driver temperature
//     0-8: temperature (60 - 90)
//   Byte 4: passenger temperature
//     0-8: temperature (60 - 90)
//   Bytes 5-8: Always 0
class RealDashListener : public FrameListener {
    public:
        RealDashListsner();

        // Connect to vehicle systems.
        void connect(ClimateController* climate);

        // Process a RealDash frame.
        void receive(uint32_t id, uint8_t len, byte* data);
    private:
        // Climate controller to send climate commands to.
        ClimateController* climate_; 
};

#endif  // __R51_REALDASH_H__
