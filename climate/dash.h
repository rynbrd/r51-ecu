#ifndef __R51_DASH_H__
#define __R51_DASH_H__

#include "controller.h"

// Controls a connected climate control system.
class DashClimateController : public Controller {
    public:
        DashClimateController() {}
        virtual ~DashClimateController() {}

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

        // Update ste of the fan speed.
        virtual void setClimateFanSpeed(uint8_t value) = 0;

        // Update the driver temperature state.
        virtual void setClimateDriverTemp(uint8_t value) = 0;

        // Update the passenger temperature state.
        virtual void setClimatePassengerTemp(uint8_t value) = 0;

        // Update the outside temperature state.
        virtual void setClimateOutsideTemp(uint8_t value) = 0;
};

// Controls a connected settings system.
class DashSettingsController : public Controller {
    public:
        DashSettingsController() {}
        virtual ~DashSettingsController() {}

        enum RemoteKeyResponseLights : uint8_t {
            LIGHTS_OFF = 0,
            LIGHTS_UNLOCK = 1,
            LIGHTS_LOCK = 2,
            LIGHTS_ON = 3,
        };

        enum AutoHeadlightOffDelay : uint8_t {
            DELAY_0S = 0,
            DELAY_30S = 2,
            DELAY_45S = 3,
            DELAY_60S = 4,
            DELAY_90S = 6,
            DELAY_120S = 8,
            DELAY_150S = 10,
            DELAY_180S = 12,
        };

        enum AutoReLockTime : uint8_t {
            RELOCK_OFF = 0,
            RELOCK_1M = 1,
            RELOCK_5M = 5,
        };

        // Update Auto Interior Illumination setting. True is on, false is off.
        virtual void setAutoInteriorIllumination(bool value) = 0;

        // Update the Auto Headlight Sensitivity setting.
        virtual void setAutoHeadlightSensitivity(uint8_t value) = 0;

        // Update the Auto Headlight Off Delay setting. Value is in in seconds.
        virtual void setAutoHeadlightOffDelay(AutoHeadlightOffDelay value) = 0;

        // Update the Speed Sensing Wiper Interval setting. True is on, false
        // is off.
        virtual void setSpeedSensingWiperInterval(bool value) = 0;

        // Update the Remote Key Response Horn setting. True is on, false is
        // off.
        virtual void setRemoteKeyResponseHorn(bool value) = 0;

        // Update the Remote Key Response Lights setting.
        virtual void setRemoteKeyResponseLights(RemoteKeyResponseLights value) = 0;

        // Update the Auto Re-Lock Time setting. Value is in minutes.
        virtual void setAutoReLockTime(AutoReLockTime value) = 0;

        // Update the Selective Door Unlock setting. True is on, false is off.
        virtual void setSelectiveDoorUnlock(bool value) = 0;

        // Update the Slide Driver Seat Back On Exit setting. True is on, false is off.
        virtual void setSlideDriverSeatBackOnExit(bool value) = 0;
};

#endif  // __R51_DASH_H__
