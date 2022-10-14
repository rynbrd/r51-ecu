#ifndef _R51_CONTROLLER_HMI_EVENT_H_
#define _R51_CONTROLLER_HMI_EVENT_H_

namespace R51 {

enum class HMIPage : uint8_t {
    SPLASH          = 0,
    HOME            = 1,
    CLIMATE         = 2,
    AUDIO           = 3,
    AUDIO_TRACK     = 4,
    AUDIO_RADIO     = 5,
    AUDIO_AUX       = 6,
    AUDIO_POWER_OFF = 7,
    AUDIO_NO_STEREO = 8,
    AUDIO_VOLUME    = 9,
    AUDIO_SOURCE    = 10,
    AUDIO_SETTINGS  = 11,
    AUDIO_EQ        = 12,
    VEHICLE         = 13,
    SETTINGS        = 14,
    SETTINGS_1      = 15,
    SETTINGS_2      = 16,
    SETTINGS_3      = 17,
    SHARED          = 18,
};

enum class HMIEvent : uint8_t {
    PAGE_STATE  = 0x00, // State event. The current page.
    SLEEP_STATE = 0x01, // State event. Sent when the display sleeps and wakes.

    // Context sensitive navigation commands.
    NAV_UP_CMD          = 0x10,
    NAV_DOWN_CMD        = 0x11,
    NAV_LEFT_CMD        = 0x12,
    NAV_RIGHT_CMD       = 0x13,
    NAV_ACTIVATE_CMD    = 0x14,
};

class DisplayPageState : public Event {
    public:
        DisplayPageState() :
            Event(SubSystem::HMI, (uint8_t)HMIEvent::PAGE_STATE, {0x00}) {}

        EVENT_PROPERTY(HMIPage, page, (HMIPage)data[0], data[0] = (uint8_t)value);
};

class DisplaySleepState : public Event {
    public:
        DisplaySleepState() :
            Event(SubSystem::HMI, (uint8_t)HMIEvent::PAGE_STATE, {0x00}) {}

        EVENT_PROPERTY(bool, sleep, data[0] == 0x01, data[0] = (uint8_t)value);
};

}  // namespace R51

#endif  // _R51_CONTROLLER_HMI_EVENT_H_
