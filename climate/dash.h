#ifndef __R51_DASH_H__
#define __R51_DASH_H__

#include "controller.h"

// Controls a connected climate control system.
class DashController : public Controller {
    public:
        DashController() {}
        virtual ~DashController() {}

        // Update the on/off state of the climate control.
        virtual void setClimateActive(bool value) = 0;

        // Update the auto setting of the climate control.
        virtual void setClimateAuto(bool value) = 0;

        // Update the state of the A/C compressor.
        virtual void setClimateAc(bool value) = 0;

        // Update dual zone state.
        virtual void setClimateDual(bool value) = 0;

        // Update state of the face vents.
        virtual void setClimateFace(bool value) = 0;

        // Update state of the feet vents.
        virtual void setClimateFeet(bool value) = 0;

        // Update state of air recirculation.
        virtual void setClimateRecirculate(bool value) = 0;

        // Update state of the front defrost.
        virtual void setClimateFrontDefrost(bool value) = 0;

        // Update state of the rear defrost.
        virtual void setClimateRearDefrost(bool value) = 0;

        // Update state of the mirror defrost.
        virtual void setClimateMirrorDefrost(bool value) = 0;

        // Update ste of the fan speed.
        virtual void setClimateFanSpeed(uint8_t value) = 0;

        // Update the driver temperature state.
        virtual void setClimateDriverTemp(uint8_t value) = 0;

        // Update the passenger temperature state.
        virtual void setClimatePassengerTemp(uint8_t value) = 0;
};

#endif  // __R51_DASH_H__
