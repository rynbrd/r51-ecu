#ifndef _R51_VEHICLE_CLIMATE_FRAMES_H_
#define _R51_VEHICLE_CLIMATE_FRAMES_H_

#include <Arduino.h>
#include <Canny.h>

namespace R51 {

// Manages changes to the 0x540 CAN frame for controlling climate state. This
// works by toggling bits in the control frame. Independent changes can be made
// before sending the frame. However, changing the same setting twice without
// sending a frame is a noop for that setting.
//
// The control frame starts in an init state. It must sends the init frame a
// few times before becoming ready.
class ClimateSystemControlFrame : public Canny::Frame {
    public:
        // Construct a climate system control object. If ready is true then the
        // control frame will immediately be placed into the ready state.
        ClimateSystemControlFrame(bool ready=false);

        // Place the control frame into the ready state. This is a noop if
        // ready() was already called.
        void ready();

        // Turn off the climate control system.
        // This is a noop until ready() is called.
        void turnOff();

        // Toggle auto mode.
        // This is a noop until ready() is called.
        void toggleAuto();

        // Toggle A/C compressor.
        // This is a noop until ready() is called.
        void toggleAC();

        // Toggle dual zone mode.
        // This is a noop until ready() is called.
        void toggleDual();

        // Cycle modes.
        // This is a noop until ready() is called.
        void cycleMode();

        // Toggle defog mode.
        // This is a noop until ready() is called.
        void toggleDefog();

        // Increase driver zone temperature. Temperature changes should not be
        // sent while the climate control system is in the off state. 
        // This is a noop until ready() is called.
        void incDriverTemp();

        // Decrease driver zone temperature. Temperature changes should not be
        // sent while the climate control system is in the off state. 
        // This is a noop until ready() is called.
        void decDriverTemp();

        // Increase passenger zone temperature. Temperature changes should not
        // be sent while the climate control system is in the off state. 
        // This is a noop until ready() is called.
        void incPassengerTemp();

        // Decrease passenger zone temperature. Temperature changes should not
        // be sent while the climate control system is in the off state. 
        // This is a noop until ready() is called.
        void decPassengerTemp();
};

// Manages changes to the 0x541 CAN frame for controlling climate fan speed and
// recirculation. This works by toggling bits in the control frame. Independent
// changes can be made before sending the frame. However, changing the same
// setting twice without sending a frame is a noop for that setting.
//
// The control frame starts in an init state. It must sends the init frame a
// few times before becoming ready.
class ClimateFanControlFrame : public Canny::Frame {
    public:
        // Construct a climate fan control object. If ready is true then the
        // control frame will immediately be placed into the ready state.
        ClimateFanControlFrame(bool ready=false);

        // Place the control frame into the ready state. This is a noop if
        // ready() was already called.
        void ready();

        // Toggle cabin recirculation.
        // This is a noop until ready() is called.
        void toggleRecirculate();

        // Increase fan speed.
        // This is a noop until ready() is called.
        void incFanSpeed();

        // Decrease fan speed.
        // This is a noop until ready() is called.
        void decFanSpeed();
};

}  // namespace R51

#endif  // _R51_VEHICLE_CLIMATE_FRAMES_H_
