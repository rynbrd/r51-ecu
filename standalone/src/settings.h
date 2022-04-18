#ifndef __R51_SETTINGS_H__
#define __R51_SETTINGS_H__

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include <Faker.h>
#include <NissanR51.h>

#include "events.h"


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
class Settings : public Caster::Node<Canny::Frame> {
    public:
        Settings(Faker::Clock* clock = Faker::Clock::real());

        // Send a frame to the node.
        void handle(const Canny::Frame& frame) override;

        // Recieve frames from the node.
        void emit(const Caster::Yield<Canny::Frame>& yield) override;

        // Exchange init frames with BCM. 
        bool init();

    private:
        Faker::Clock* clock_;
        bool state_changed_;
        uint32_t state_last_broadcast_;
        Canny::Frame state_;
        byte control_state_[8];
        NissanR51::Settings settings_;

        void handleControl(const Canny::Frame& frame);
};

#endif  // __R51_SETTINGS_H__
