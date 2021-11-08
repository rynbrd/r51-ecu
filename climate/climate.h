#ifndef __R51_CLIMATE_H__
#define __R51_CLIMATE_H__

#include "controller.h"

// Controls a climate control system.
class ClimateController : public Controller {
    public:
        ClimateController() {}
        virtual ~ClimateController() {}

        // Send a climate "Off" button click. This typically turns off the
        // climate control system.
        virtual void climateClickOff() = 0;

        // Send a climate "Auto" button click. This typically toggles automatic
        // climate control.
        virtual void climateClickAuto() = 0;

        // Send a climate "A/C" button click. This typically toggles the A/C
        // compressor request.
        virtual void climateClickAc() = 0;

        // Send a climate "Dual" button click. This typically toggles dual zone
        // climate control. 
        virtual void climateClickDual() = 0;

        // Send a climate "Recirculate" button click. This typically toggles
        // cabin air recirculation.
        virtual void climateClickRecirculate() = 0;

        // Send a climate "Mode" button click. This typically cycles the
        // airflow mode.
        virtual void climateClickMode() = 0;

        // Send a climate "Front Defrost" button click. This typically toggles
        // windshield defrost.
        virtual void climateClickFrontDefrost() = 0;

        // Send a climate "Rear Defrost" button click. This typically toggles
        // rear window defrost.
        virtual void climateClickRearDefrost() = 0;

        // Send a climate "Fan Up" button click. This typically increases the
        // fan speed by 1.
        virtual void climateClickFanSpeedUp() = 0;

        // Send a climate "Fan Down" button click. This typically decreases the
        // fan speed by 1.
        virtual void climateClickFanSpeedDown() = 0;

        // Send a "Driver Temperature Up" button click. This typically
        // increases the driver temperature zone by 1.
        virtual void climateClickDriverTempUp() = 0;

        // Send a "Driver Temperature Down" button click. This typically
        // decreases the driver temperature zone by 1.
        virtual void climateClickDriverTempDown() = 0;

        // Send a "Passenger Temperature Up" button click. This typically
        // increases the passenger temperature zone by 1.
        virtual void climateClickPassengerTempUp() = 0;

        // Send a "Passenger Temperature Down" button click. This typically
        // decreases the passenger temperature zone by 1.
        virtual void climateClickPassengerTempDown() = 0;
};

#endif  // __R51_CLIMATE_H__
