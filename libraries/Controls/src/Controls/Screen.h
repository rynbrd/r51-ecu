#ifndef _R51_CONTROLS_SCREEN_H_
#define _R51_CONTROLS_SCREEN_H_

namespace R51 {

enum class ScreenPage : uint8_t {
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
    BLANK           = 19,
};

enum class ScreenEvent : uint8_t {
    POWER_STATE = 0x00, // State event. Sent when the display powers
                        // on/off or changes brightness.
    PAGE_STATE  = 0x01, // State event. The current page.

    // Display power controls.
    POWER_CMD       = 0x10, // Turn the display on/off.
    BRIGHTNESS_CMD  = 0x11, // Set the brightness of the display.

    // Context sensitive navigation commands.
    NAV_UP_CMD          = 0x20,
    NAV_DOWN_CMD        = 0x21,
    NAV_LEFT_CMD        = 0x22,
    NAV_RIGHT_CMD       = 0x23,
    NAV_ACTIVATE_CMD    = 0x24,
    NAV_HOME_CMD        = 0x25, // Move to the home page.
    NAV_PAGE_NEXT_CMD   = 0x26, // Move to the next page. Rotates between
                                // climate, audio, and vehicle
    NAV_PAGE_PREV_CMD   = 0x27, // Move to the previous page. Rotates between
                                // vehicle, audio, and climate.
};

class ScreenPageState : public Event {
    public:
        ScreenPageState() :
            Event(SubSystem::SCREEN, (uint8_t)ScreenEvent::PAGE_STATE, {0x00}) {}

        EVENT_PROPERTY(ScreenPage, page, (ScreenPage)data[0], data[0] = (uint8_t)value);
};

class ScreenPowerState : public Event {
    public:
        ScreenPowerState() :
            Event(SubSystem::SCREEN, (uint8_t)ScreenEvent::POWER_STATE, {0x01, 0xFF}) {}

        EVENT_PROPERTY(bool, power, data[0] != 0x00, data[0] = (bool)value);
        EVENT_PROPERTY(uint8_t, brightness, data[1], data[1] = value);
};

}  // namespace R51

#endif  // _R51_CONTROLS_SCREEN_H_
