#ifndef __R51_CLIMATE_H__
#define __R51_CLIMATE_H__

#include <Arduino.h>
#include <Canny.h>
#include <Faker.h>
#include <NissanR51.h>

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

        // Update the climate state from vehicle state frames. Only handles
        // vehicle and dash frames: 
        //   Vehicle: 0x54A, 0x54B, 0x625
        //   Dash:    0x5401
        void handle(const Canny::Frame& frame) override;

        // Receive translated state frames.
        void emit(const Yield& yield) override;

    private:
        Faker::Clock* clock_;

        // Hardware control.
        MomentaryOutput rear_defrost_;

        // CAN frame state parsing.
        NissanR51::ClimateSystemState system_state_;
        NissanR51::ClimateTemperatureState temp_state_;

        // State frame storage.
        uint8_t state_init_;
        bool state_changed_;
        uint32_t state_last_broadcast_;
        Canny::Frame state_frame_;

        // Control frame storage.
        bool control_init_;
        uint32_t control_last_broadcast_;
        NissanR51::ClimateSystemControl system_control_;
        NissanR51::ClimateFanControl fan_control_;
        Canny::Frame control_frame_540_;
        Canny::Frame control_frame_541_;
        byte control_state_[8];

        // Specific frame handlers.
        void handleTemps(const Canny::Frame& frame);
        void handleSystem(const Canny::Frame& frame);
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
        void setMode(NissanR51::ClimateVents vents);
};

#endif  // __R51_CLIMATE_H__
