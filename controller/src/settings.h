#ifndef __R51_SETTINGS_H__
#define __R51_SETTINGS_H__

#include "controller.h"


// Controls the BCM settings.
class SettingsController : public Controller {
    public:
        // Toggle the Auto Interior Illumination setting.
        virtual bool toggleAutoInteriorIllumination() = 0;

        // Cycle the Auto Headlight Sensitivity setting to the next value.
        virtual bool nextAutoHeadlightSensitivity() = 0;

        // Cycle the Auto Headlight Sensitivity setting to the previous value.
        virtual bool prevAutoHeadlightSensitivity() = 0;

        // Cycle the Auto Headlight Off Delay to the next setting.
        virtual bool nextAutoHeadlightOffDelay() = 0;

        // Cycle the Auto Headlight Off Delay to the previous setting.
        virtual bool prevAutoHeadlightOffDelay() = 0;

        // Toggle the Speed Sensing Wiper Interval setting.
        virtual bool toggleSpeedSensingWiperInterval() = 0;

        // Toggle the Remote Key Response Horn setting.
        virtual bool toggleRemoteKeyResponseHorn() = 0;

        // Cycle the Remote Key Response Lights setting to the next value.
        virtual bool nextRemoteKeyResponseLights() = 0;

        // Cycle the Remote Key Response Lights setting to the previous value.
        virtual bool prevRemoteKeyResponseLights() = 0;

        // Cycle the Auto Re-Lock Time setting to the next value.
        virtual bool nextAutoReLockTime() = 0;

        // Cycle the Auto Re-Lock Time setting to the previous value.
        virtual bool prevAutoReLockTime() = 0;

        // Toggle the Selective Door Unlock setting.
        virtual bool toggleSelectiveDoorUnlock() = 0;

        // Toggle the Slide Driver Seat Back On Exit setting.
        virtual bool toggleSlideDriverSeatBackOnExit() = 0;

        // Retrieve the current settings.
        virtual bool retrieveSettings() = 0;

        // Reset all settings back to their defaults.
        virtual bool resetSettingsToDefault() = 0;
};

#endif  // __R51_SETTINGS_H__
