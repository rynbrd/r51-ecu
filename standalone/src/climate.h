#ifndef __R51_CLIMATE_H__
#define __R51_CLIMATE_H__

#include <Arduino.h>
#include <Canny.h>
#include <Faker.h>

#include "bus.h"
#include "momentary_output.h"


// Manages vehicle climate control system and sends state changes via a single
// frame.
//
// On start it is in the "initialization" state and will send some
// initialization control frames to the A/C Auto Amp to bring it online. The
// controller will send control frames at least every 200ms to ensure the A/C
// Auto Amp remains active.
//
// Frame 0x5400: Climate State Frame
//   Byte 0: Unit and Vent State
//     Bit 0: active
//     Bit 1: auto
//     Bit 2: ac
//     Bit 3: dual
//     Bit 4: face
//     Bit 5: feet
//     Bit 6: defrost (windshield)
//     Bit 7: recirculate
//   Byte 1: Fan Speed
//   Byte 2: Driver Temperature
//   Byte 3: Passenger Temperature
//   Byte 4: Heating Elements
//     Bit 0: rear window heating element
//     Bit 1-7: unused
//   Bytes 5-6: unused
//   Byte 7: Outside Temperature
//
// Frame 0x5401: Climate Control Frame
//   Byte 0: Unit State and Mode
//     Bit 0: turn off
//     Bit 1: toggle auto
//     Bit 2: toggle ac
//     Bit 3: toggle dual
//     Bit 4: cycle mode
//     Bit 5: unused
//     Bit 6: toggle defrost (windshield)
//     Bit 7: toggle recirculate
//   Byte 1: Fan and Temperature Control
//     Bit 0: fan speed +
//     Bit 1: fan speed -
//     Bit 2: driver temp + 
//     Bit 3: driver temp -
//     Bit 4: passenger temp +
//     Bit 5: passenger temp -
//     Bit 6-7: unused
//   Byte 2: Driver Temperature Set
//   Byte 3: Passenger Temperature Set
//   Byte 4: Heating Elements
//     Bit 0: toggle rear window heating element
//     Bit 1-7: unused
//   Bytes 5-7: unused
class Climate : public Node {
    public:
        Climate(Faker::Clock* clock = Faker::Clock::real(),
                Faker::GPIO* gpio = Faker::GPIO::real());

        // Receive translated state frames.
        void receive(const Broadcast& broadcast) override;

        // Update the climate state from vehicle state frames.
        void send(const Canny::Frame& frame) override;

        // Matches vehicle state frames and dash control frames.
        //   Vehicle: 0x54A, 0x54B, 0x625
        //   Dash:    0x5401
        bool filter(const Canny::Frame& frame) const override;

    private:
        Faker::Clock* clock_;

        // Hardware control.
        MomentaryOutput rear_defrost_;

        // Operational state.
        enum State : uint8_t {
            STATE_OFF,
            STATE_AUTO,
            STATE_MANUAL,
            STATE_HALF_MANUAL,
            STATE_DEFROST,
        };
        enum Mode : uint8_t {
            MODE_OFF = 0x00,
            MODE_FACE = 0x04,
            MODE_FACE_FEET = 0x08,
            MODE_FEET = 0x0C,
            MODE_FEET_WINDSHIELD = 0x10,
            MODE_WINDSHIELD = 0x34,
            MODE_AUTO_FACE = 0x84,
            MODE_AUTO_FACE_FEET = 0x88,
            MODE_AUTO_FEET = 0x8C,
        };
        State state_;
        Mode mode_;
        bool dual_;

        // State frame storage.
        uint8_t state_init_;
        bool state_changed_;
        uint32_t state_last_broadcast_;
        Canny::Frame state_frame_;

        // Control frame storage.
        bool control_init_;
        bool control_changed_;
        uint32_t control_last_broadcast_;
        Canny::Frame control_frame_540_;
        Canny::Frame control_frame_541_;
        byte control_state_[8];

        // Specific frame handlers.
        void handle54A(const Canny::Frame& frame);
        void handle54B(const Canny::Frame& frame);
        void handle625(const Canny::Frame& frame);
        void handleControl(const Canny::Frame& frame);

        // Helpers for setting climate state.
        void setActive(bool value);
        void setAuto(bool value);
        void setAc(bool value);
        void setDual(bool value);
        void setFace(bool value);
        void setFeet(bool value);
        void setRecirculate(bool value);
        void setFrontDefrost(bool value);
        void setRearDefrost(bool value);
        void setFanSpeed(uint8_t value);
        void setDriverTemp(uint8_t value);
        void setPassengerTemp(uint8_t value);
        void setOutsideTemp(uint8_t value);
        void setMode(uint8_t mode);

        // Helpers for triggering climate system events.
        void triggerOff();
        void triggerAuto();
        void triggerAc();
        void triggerDual();
        void triggerRecirculate();
        void triggerMode();
        void triggerFrontDefrost();
        void triggerRearDefrost();
        void triggerFanSpeedUp();
        void triggerFanSpeedDown();
        void triggerDriverTempUp();
        void triggerDriverTempDown();
        void triggerPassengerTempUp();
        void triggerPassengerTempDown();
};

#endif  // __R51_CLIMATE_H__
