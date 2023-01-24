#ifndef _R51_CONTROLS_AUDIO_H_
#define _R51_CONTROLS_AUDIO_H_

#include <Arduino.h>
#include <ByteOrder.h>
#include <Core.h>
#include <Foundation.h>

namespace R51 {

static const uint8_t kVolumeMax = 24;
static const int8_t kBalanceMin = -7;
static const int8_t kBalanceMax = 7;
static const int8_t kFadeMin = -8;
static const int8_t kFadeMax = 8;
static const int8_t kToneMin = -15;
static const int8_t kToneMax = 15;

// System state of the audio device.
enum class AudioSystem : uint8_t {
    UNAVAILABLE = 0x00, // No audio system is connected.
    OFF         = 0x01, // System is connected but powered down.
    BOOT        = 0x02, // Audio system is initializing.
    ON          = 0x03, // System is on.
    POWER_ON    = 0x04, // Audio system is powering on.
};

// Audio source.
enum class AudioSource : uint8_t {
    AM = 0x00,
    FM = 0x01,
    //SIRIUSXM,
    AUX = 0x03,
    //USB,
    //IPOD,
    //MTP,
    BLUETOOTH = 0x07,
    //DAB,
    OPTICAL = 0x09,
    //AIRPLAY,
    //UPNP,
};

// Playback state for Bluetooth.
enum class AudioPlayback : uint8_t {
    NO_TRACK = 0x00,
    PLAY = 0x01,
    PAUSE = 0x02,
};

// Seek mode for broadcast radio.
enum class AudioSeek : uint8_t {
    AUTO = 0x00,
    MANUAL = 0x01,
};

enum class AudioEvent {
    // State events.
    SYSTEM_STATE            = 0x00, // System availability and power status.
    VOLUME_STATE            = 0x01, // Volume, fade, and balance.
    TONE_STATE              = 0x02, // Equalizer high/mid/low settings.
    SOURCE_STATE            = 0x03, // Current source and bluetooth connectivity.
    TRACK_PLAYBACK_STATE    = 0x04, // Playback status and track times.
    TRACK_TITLE_STATE       = 0x05, // Track title. Shared via scratch pointer.
    TRACK_ARTIST_STATE      = 0x06, // Artist name. Shared via scratch pointer.
    TRACK_ALBUM_STATE       = 0x07, // Album title. Shared via scratch pointer.
    RADIO_STATE             = 0x08, // Seek mode and frequency.
    INPUT_STATE             = 0x09, // Aux/Optical gain.

    // System controls.
    POWER_ON_CMD        = 0x10, // Turn on the stereo.
    POWER_OFF_CMD       = 0x11, // Turn off the stereo.
    POWER_TOGGLE_CMD    = 0x12, // Toggle stereo power.
    SOURCE_SET_CMD      = 0x13, // Set source.
    SOURCE_NEXT_CMD     = 0x14, // Next source. Only rotates through AM, FM, and BT.
    SOURCE_PREV_CMD     = 0x15, // Previous source. Only rotates through AM, FM, and BT.

    // Track controls.
    TRACK_PLAY_CMD  = 0x20, // Play current track.
    TRACK_PAUSE_CMD = 0x21, // Pause current track.
    TRACK_NEXT_CMD  = 0x22, // Next track.
    TRACK_PREV_CMD  = 0x23, // Prev track.

    // Radio controls.
    RADIO_TUNE_CMD          = 0x30, // Tune radio to a specific frequency.
    RADIO_NEXT_AUTO_CMD     = 0x31, // Next radio frequency in auto seek mode.
    RADIO_PREV_AUTO_CMD     = 0x32, // Prev radio frequency in auto seek mode.
    RADIO_NEXT_MANUAL_CMD   = 0x33, // Next radio frequency in manual seek mode.
    RADIO_PREV_MANUAL_CMD   = 0x34, // Prev radio frequency in manual seek mode.
    RADIO_TOGGLE_SEEK_CMD   = 0x35, // Toggle seek mode.
    RADIO_NEXT_CMD          = 0x36, // Next radio frequency in current seek mode.
    RADIO_PREV_CMD          = 0x37, // Next radio frequency in current seek mode.

    // Input controls.
    INPUT_GAIN_SET_CMD  = 0x40, // Set the input gain to specific value.
    INPUT_GAIN_INC_CMD  = 0x41, // Increment the input gain value.
    INPUT_GAIN_DEC_CMD  = 0x42, // Decrement the input gain value.

    // Volume controls.
    // TODO: allow inc/dec by a provided amount
    VOLUME_SET_CMD          = 0x50, // Set volume.
    VOLUME_INC_CMD          = 0x51, // Increment volume.
    VOLUME_DEC_CMD          = 0x52, // Decrement volume.
    VOLUME_MUTE_CMD         = 0x53, // Mute volume.
    VOLUME_UNMUTE_CMD       = 0x54, // Unmute volume.
    VOLUME_TOGGLE_MUTE_CMD  = 0x55, // Toggle volume mute.
    BALANCE_SET_CMD         = 0x56, // Set the audio balance.
    BALANCE_LEFT_CMD        = 0x57, // Shift audio balance to the left.
    BALANCE_RIGHT_CMD       = 0x58, // Shift audio balance to the right.
    FADE_SET_CMD            = 0x59, // Set the audio fade.
    FADE_FRONT_CMD          = 0x5A, // Fade audio to the front.
    FADE_REAR_CMD           = 0x5B, // Fade audio to the back.

    // EQ controls.
    TONE_SET_CMD        = 0x60, // Set bass, mid, and treble.
    TONE_BASS_INC_CMD   = 0x61, // Increase bass.
    TONE_BASS_DEC_CMD   = 0x62, // Decrease bass.
    TONE_MID_INC_CMD    = 0x63, // Increase mid.
    TONE_MID_DEC_CMD    = 0x64, // Decrease mid.
    TONE_TREBLE_INC_CMD = 0x65, // Increase treble.
    TONE_TREBLE_DEC_CMD = 0x66, // Decrease treble.

    // Stateless playback controls.
    PLAYBACK_TOGGLE_CMD = 0xE0, // Toggles between play/pause for Bluetooth
                                // or seek mode for radio.
    PLAYBACK_NEXT_CMD   = 0xE1, // Advance to next track for Bluetooth or auto
                                // seek to next station for radio.
    PLAYBACK_PREV_CMD   = 0xE2, // Go to previous track for Bluetooth or auto
                                // seek to next station for radio.

    // Settings menu events and controls.
    SETTINGS_OPEN_CMD   = 0xF0, // Sent by the user to open the settings menu.
    SETTINGS_BACK_CMD   = 0xF1, // Sent by the user to go the the previous settings menu.
    SETTINGS_EXIT_CMD   = 0xF2, // Sent by the user to exit the settings menu.
    SETTINGS_SELECT_CMD = 0xF3, // Sent by the user to select a settings menu item.
    SETTINGS_MENU_STATE = 0x0A, // Sent by Fusion to display a settings menu.
    SETTINGS_ITEM_STATE = 0x0B, // Sent by Fusion to display an item in a settings menu.
    SETTINGS_EXIT_STATE = 0x0C, // Sent by Fusion to stop displaying the settings menu.
};

class AudioSystemState : public Event {
    public:
        AudioSystemState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SYSTEM_STATE, {0x00}) {}

        EVENT_PROPERTY(AudioSystem, state, (AudioSystem)data[0], data[0] = (uint8_t)value);
};

class AudioVolumeState : public Event {
    public:
        AudioVolumeState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::VOLUME_STATE,
                    {0x00, 0x00, 0x00, 0x00}) {}

        EVENT_PROPERTY(uint8_t, volume, data[0], data[0] = value);
        EVENT_PROPERTY(int8_t, fade, (int8_t)data[1], data[1] = (uint8_t)value);
        EVENT_PROPERTY(int8_t, balance, (int8_t)data[2], data[2] = (uint8_t)value);
        EVENT_PROPERTY(bool, mute, data[3] != 0x00, data[3] = (uint8_t)value);
};

class AudioToneState : public Event {
    public:
        AudioToneState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::TONE_STATE,
                    {0x00, 0x00, 0x00}) {}

        EVENT_PROPERTY(int8_t, bass, (int8_t)data[0], data[0] = (uint8_t)value);
        EVENT_PROPERTY(int8_t, mid, (int8_t)data[1], data[1] = (uint8_t)value);
        EVENT_PROPERTY(int8_t, treble, (int8_t)data[2], data[2] = (uint8_t)value);
};

class AudioSourceState : public Event {
    public:
        AudioSourceState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SOURCE_STATE, {0x00}) {}

        EVENT_PROPERTY(AudioSource, source, (AudioSource)data[0], data[0] = (uint8_t)value);
        EVENT_PROPERTY(bool, bt_connected, data[1] != 0x00, data[1] = (uint8_t)value);
};

class AudioTrackPlaybackState : public Event {
    public:
        AudioTrackPlaybackState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::TRACK_PLAYBACK_STATE,
                    {0x00, 0x00, 0x00, 0x00, 0x00}) {}

        EVENT_PROPERTY(AudioPlayback, playback, (AudioPlayback)data[0], data[0] = (uint8_t)value);
        EVENT_PROPERTY(uint16_t, time_elapsed,
                ByteOrder::nbtohs(data + 1),
                ByteOrder::hstonb(data + 1, value));
        EVENT_PROPERTY(uint16_t, time_total,
                ByteOrder::nbtohs(data + 3),
                ByteOrder::hstonb(data + 3, value));
};

class AudioSettingsMenuState : public Event {
    public:
        AudioSettingsMenuState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SETTINGS_MENU_STATE,
                    {0x00, 0x00}) {}

        EVENT_PROPERTY(uint8_t, page, data[0], data[0] = value);
        EVENT_PROPERTY(uint8_t, item, data[1], data[1] = value);
        EVENT_PROPERTY(uint8_t, count, data[2], data[2] = value);
};

enum class AudioSettingsType : uint8_t {
    SUBMENU = 1,
    SELECT = 2,
    CHECKBOX_OFF = 3,
    CHECKBOX_ON = 4,
};

class AudioSettingsItemState : public Event {
    public:
        AudioSettingsItemState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SETTINGS_ITEM_STATE,
                    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}) {}

        EVENT_PROPERTY(uint8_t, item,
                data[1] & 0x0F,
                data[1] = (data[1] & 0xF0) | (value & 0x0F));
        EVENT_PROPERTY(bool, reload,
                getBit(data, 1, 5),
                setBit(data, 1, 5, value));
        EVENT_PROPERTY(AudioSettingsType, type,
                (AudioSettingsType)data[2],
                data[2] = (uint8_t)value);
};

class AudioSettingsExitState : public Event {
    public:
        AudioSettingsExitState() :
                Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SETTINGS_ITEM_STATE) {}
};

class AudioSettingsOpenCommand : public Event {
    public:
        AudioSettingsOpenCommand() :
                Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SETTINGS_OPEN_CMD) {}
};

class AudioSettingsSelectCommand : public Event {
    public:
        AudioSettingsSelectCommand(uint8_t item = 0x00) :
                Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SETTINGS_SELECT_CMD, {item}) {}

        EVENT_PROPERTY(uint8_t, item, data[0], data[0] = value);
};

class AudioSettingsBackCommand : public Event {
    AudioSettingsBackCommand() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SETTINGS_BACK_CMD) {}
};

class AudioSettingsExitCommand : public Event {
    AudioSettingsExitCommand() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SETTINGS_EXIT_CMD) {}
};

class AudioTrackTitleState : public Event {
    public:
        AudioTrackTitleState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::TRACK_TITLE_STATE) {}
};

class AudioTrackArtistState : public Event {
    public:
        AudioTrackArtistState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::TRACK_ARTIST_STATE) {}
};

class AudioTrackAlbumState : public Event {
    public:
        AudioTrackAlbumState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::TRACK_ALBUM_STATE) {}
};

class AudioRadioState : public Event {
    public:
        AudioRadioState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::RADIO_STATE,
                {0x00, 0x00, 0x00, 0x00, 0x00}) {}

        EVENT_PROPERTY(AudioSeek, seek_mode,
                (AudioSeek)getBit(data, 0, 0),
                setBit(data, 0, 0, (bool)value));
        EVENT_PROPERTY(uint32_t, frequency,
                ByteOrder::nbtohl(data + 1),
                ByteOrder::hltonb(data + 1, value));

        void toggle_seek_mode() {
            flipBit(data, 0, 0);
        }
};


class AudioInputState : public Event {
    public:
        AudioInputState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::INPUT_STATE, {0x00}) {}

        EVENT_PROPERTY(int8_t, gain, (int8_t)data[0], data[0] = (uint8_t)value);
};

class AudioSourceSetCommand : public Event {
    public:
        AudioSourceSetCommand() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SOURCE_SET_CMD, {0x00}) {}

        EVENT_PROPERTY(AudioSource, source,
                (AudioSource)data[0],
                data[0] = (uint8_t)value);
};

class AudioRadioTuneCommand : public Event {
    public:
        AudioRadioTuneCommand() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::RADIO_TUNE_CMD,
                    {0x00, 0x00, 0x00, 0x00}) {}

        EVENT_PROPERTY(uint32_t, frequency,
                ByteOrder::nbtohl(data),
                ByteOrder::hltonb(data, value));
};

class AudioInputGainSetCommand : public Event {
    public:
        AudioInputGainSetCommand() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::INPUT_GAIN_SET_CMD, {0x00}) {}

        EVENT_PROPERTY(int8_t, gain, (int8_t)data[0], data[0] = (uint8_t)value);
};

class AudioVolumeSetCommand : public Event {
    public:
        AudioVolumeSetCommand() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::VOLUME_SET_CMD, {0x00}) {}

        EVENT_PROPERTY(uint8_t, volume, data[0], data[0] = value);
};

class AudioBalanceSetCommand : public Event {
    public:
        AudioBalanceSetCommand() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::BALANCE_SET_CMD, {0x00}) {}

        EVENT_PROPERTY(int8_t, balance, (int8_t)data[0], data[0] = (uint8_t)value);
};

class AudioFadeSetCommand : public Event {
    public:
        AudioFadeSetCommand() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::BALANCE_SET_CMD, {0x00}) {}

        EVENT_PROPERTY(int8_t, fade, (int8_t)data[0], data[0] = (uint8_t)value);
};

class AudioToneSetCommand : public Event {
    public:
        AudioToneSetCommand() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::TONE_SET_CMD,
                    {0x00, 0x00, 0x00}) {}

        EVENT_PROPERTY(int8_t, bass, (int8_t)data[0], data[0] = (uint8_t)value);
        EVENT_PROPERTY(int8_t, mid, (int8_t)data[1], data[1] = (uint8_t)value);
        EVENT_PROPERTY(int8_t, treble, (int8_t)data[2], data[2] = (uint8_t)value);
};

}  // namespace R51

#endif  // _R51_CONTROLS_AUDIO_H_
