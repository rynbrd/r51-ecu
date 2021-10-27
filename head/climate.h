#ifndef __R51_CLIMATE_H__
#define __R51_CLIMATE_H__

#include "controller.h"

// Controls a climate control system.
class ClimateController : public Controller {
    public:
        ClimateController() {}
        virtual ~ClimateController() {}

        // Turn off the climate control.
        virtual void deactivateClimate() = 0;

        // Toggle the climate control auto setting.
        virtual void toggleClimateAuto() = 0;

        // Toggle the climate control A/C setting.
        virtual void toggleClimateAc() = 0;

        // Toggle the climate control dual zone setting.
        virtual void toggleClimateDual() = 0;

        // Toggle climate control air recirculation.
        virtual void toggleClimateRecirculate() = 0;

        // Cycle the climate control airflow mode.
        virtual void cycleClimateMode() = 0;

        // Toggle front defrost. This toggles windshield airflow.
        virtual void toggleClimateFrontDefrost() = 0;

        // Toggle rear defrost. This toggles the rear window heating element.
        virtual void toggleClimateRearDefrost() = 0;

        // Increase the climate control fan speed by one notch.
        virtual void increaseClimateFanSpeed() = 0;

        // Decrease the climate control fan speed by one notch.
        virtual void decreaseClimateFanSpeed() = 0;

        // Set the driver climate zone temperature in degrees from 60 to 90.
        virtual void setClimateDriverTemp(uint8_t temp) = 0;

        // Set the passenger climate zone temperature in degrees from 60 to 90.
        virtual void setClimatePassengerTemp(uint8_t temp) = 0;
};

#endif  // __R51_CLIMATE_H__
