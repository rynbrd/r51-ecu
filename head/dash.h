#ifndef __R51_DASH_H__
#define __R51_DASH_H__

// Controls a connected climate control system.
class DashController {
    public:
        DashController() {}
        virtual ~DashController() {}

        // Update the on/off state of the climate control.
        virtual void setClimateActive(bool value) {}

        // Update the auto setting of the climate control.
        virtual void setClimateAuto(bool value) {}

        // Update the state of the A/C compressor.
        virtual void setClimateAc(bool value) {}

        // Update dual zone state.
        virtual void setClimateDual(bool value) {}

        // Update state of the face vents.
        virtual void setClimateFace(bool value) {}

        // Update state of the feet vents.
        virtual void setClimateFeet(bool value) {}

        // Update state of air recirculation.
        virtual void setClimateRecirculate(bool value) {}

        // Update state of the front defrost.
        virtual void setClimateFrontDefrost(bool value) {}

        // Update state of the rear defrost.
        virtual void setClimateRearDefrost(bool value) {}

        // Update ste of the fan speed.
        virtual void setClimateFanSpeed(uint8_t value) {}

        // Update the driver temperature state.
        virtual void setClimateDriverTemp(uint8_t value) {}

        // Update the passenger temperature state.
        virtual void setClimatePassengerTemp(uint8_t value) {}
};

#endif  // __R51_DASH_H__
