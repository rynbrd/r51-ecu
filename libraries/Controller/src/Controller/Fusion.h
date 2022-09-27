#ifndef _R51_CONTROLLER_FUSION_H_
#define _R51_CONTROLLER_FUSION_H_

#include <Arduino.h>
#include <CRC32.h>
#include <Caster.h>
#include <Common.h>
#include <Endian.h>
#include <Faker.h>

namespace R51 {

enum class AudioSource : uint8_t {
    AM = 0x00,
    FM = 0x01,
    SIRIUSXM = 0x02,
    AUX = 0x03,
    //USB,
    //IPOD,
    //MTP,
    BLUETOOTH = 0x07,
    //DAB,
    OPTICAL = 0x09,
    AIRPLAY = 0x0A,
    //UPNP,
};

enum class AudioPlayback : uint8_t {
    NO_TRACK = 0x00,
    PLAY = 0x01,
    PAUSE = 0x02,
    NO_DEVICE = 0xFF,
};

enum class AudioEvent {
    // State events.
    SYSTEM_STATE            = 0x00,  // State event. Sends power, connectivity, gain, frequency.
    VOLUME_STATE            = 0x01,  // State event. Sends current volume, fade, and balance.
    MUTE_STATE              = 0x02,  // State event. Sent on mute/unmute.
    EQ_STATE                = 0x03,  // State event. Sends equalizer high/mid/low values.
    TRACK_PLAYBACK_STATE    = 0x04,  // State event. Sends the playback status and track times.
    TRACK_TITLE_STATE       = 0x05,  // State event. Sends the track title.
    TRACK_ARTIST_STATE      = 0x06,  // State event. Sends the artist name.
    TRACK_ALBUM_STATE       = 0x07,  // State event. Sends the album title.

    // System controls.
    POWER_ON                = 0x10,  // Turn on the stereo.
    POWER_OFF               = 0x11,  // Turn off the stereo.
    SET_SOURCE              = 0x12,  // Set source.
    BT_DISCO_TOGGLE         = 0x13,  // Toggle Bluetooth discovery on/off.
    BT_CONNECT              = 0x14,  // Connect to a Bluetooth device.
    BT_FORGET               = 0x15,  // Forget a Bluetooth device.

    // Track controls.
    TRACK_PLAY              = 0x20,  // Play current track.
    TRACK_PAUSE             = 0x21,  // Pause current track.
    TRACK_NEXT              = 0x22,  // Next track.
    TRACK_PREV              = 0x23,  // Prev track.

    // Radio controls.
    RADIO_FREQ_NEXT_AUTO    = 0x30,  // Next radio frequency in auto scan mode.
    RADIO_FREQ_PREV_AUTO    = 0x31,  // Prev radio frequency in auto scan mode.
    RADIO_FREQ_NEXT_MANUAL  = 0x32,  // Next radio frequency in manual scan mode.
    RADIO_FREQ_PREV_MANUAL  = 0x33,  // Prev radio frequency in manual scan mode.

    // Input controls.
    INPUT_GAIN_INC          = 0x40,  // Increment the input gain value.
    INPUT_GAIN_DEC          = 0x41,  // Decrement the input gain value.

    // Volums and EQ controls.
    VOLUME_INC              = 0x50,  // Increment volume.
    VOLUME_DEC              = 0x51,  // Decrement volume.
    BALANCE_LEFT            = 0x52,  // Shift audio balance to the left.
    BALANCE_RIGHT           = 0x53,  // Shift audio balance to the right.
    FADE_FRONT              = 0x54,  // Fade audio to the front.
    FADE_BACK               = 0x55,  // Fade audio to the back.
    BASS_INC                = 0x56,  // Increase bass.
    BASS_DEC                = 0x57,  // Decrease bass.
    MID_INC                 = 0x58,  // Increase mid.
    MID_DEC                 = 0x59,  // Decrease mid.
    TREBLE_INC              = 0x5A,  // Increase treble.
    TREBLE_DEC              = 0x5B,  // Decrease treble.
    MUTE                    = 0x5C,  // Mute volume.
    UNMUTE                  = 0x5D,  // Unmute volume.

    // Settings menu events and controls.
    SETTINGS_CMD_OPEN       = 0xF0,  // Sent by the user to open the settings menu.
    SETTINGS_CMD_SELECT     = 0xF1,  // Sent by the user to select a settings menu item.
    SETTINGS_CMD_BACK       = 0xF2,  // Sent by the user to go the the previous settings menu.
    SETTINGS_CMD_EXIT       = 0xF3,  // Sent by the user to exit the settings menu.
    SETTINGS_STATE_MENU     = 0xF4,  // Sent by Fusion to display a settings menu.
    SETTINGS_STATE_ITEM     = 0xF5,  // Sent by Fusion to display an item in a settings menu.
    SETTINGS_STATE_EXIT     = 0xF6,  // Sent by Fusion to stop displaying the settings menu.
};

enum class AudioSettingsType : uint8_t {
    SUBMENU = 0x49,
    SELECT = 0x11,
    CHECKBOX_OFF = 0x89,
    CHECKBOX_ON = 0x8B,
};

class AudioSystemState : public Event {
    public:
        AudioSystemState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SYSTEM_STATE,
                    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}) {}

        EVENT_PROPERTY(bool, available, getBit(data, 0, 4), setBit(data, 0, 4, value));
        EVENT_PROPERTY(bool, power, getBit(data, 0, 5), setBit(data, 0, 5, value));
        EVENT_PROPERTY(bool, bt_connected, getBit(data, 0, 6), setBit(data, 0, 6, value));
        EVENT_PROPERTY(bool, bt_discoverable, getBit(data, 0, 7), setBit(data, 0, 7, value));
        EVENT_PROPERTY(AudioSource, source,
                (AudioSource)(data[0] & 0x0F),
                data[0] |= ((uint8_t)value & 0x0F));
        EVENT_PROPERTY(int8_t, gain, (int8_t)data[1], data[1] = (uint8_t)value);
        EVENT_PROPERTY(uint32_t, frequency,
                Endian::nbtohl(data + 2),
                Endian::hltonb(data + 2, value));
};


class AudioVolumeState : public Event {
    public:
        AudioVolumeState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::VOLUME_STATE,
                    {0x00, 0x00, 0x00}) {}

        EVENT_PROPERTY(uint8_t, volume, data[0], data[0] = value);
        EVENT_PROPERTY(int8_t, fade, (int8_t)data[1], data[1] = (uint8_t)value);
        EVENT_PROPERTY(int8_t, balance, (int8_t)data[2], data[2] = (uint8_t)value);
};

class AudioMuteState : public Event {
    public:
        AudioMuteState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::MUTE_STATE,
                    {0x00}) {}

        EVENT_PROPERTY(bool, mute, data[0] != 0, data[0] = (uint8_t)value);
};

class AudioToneState : public Event {
    public:
        AudioToneState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::EQ_STATE,
                    {0x00, 0x00, 0x00}) {}

        EVENT_PROPERTY(int8_t, bass, (int8_t)data[0], data[0] = (uint8_t)value);
        EVENT_PROPERTY(int8_t, mid, (int8_t)data[1], data[1] = (uint8_t)value);
        EVENT_PROPERTY(int8_t, treble, (int8_t)data[2], data[3] = (uint8_t)value);
};

class AudioTrackPlaybackState : public Event {
    public:
        AudioTrackPlaybackState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::TRACK_PLAYBACK_STATE,
                    {0x00, 0x00, 0x00, 0x00, 0x00}) {}

        EVENT_PROPERTY(AudioPlayback, playback, (AudioPlayback)data[0], data[0] = (uint8_t)value);
        EVENT_PROPERTY(uint16_t, time_elapsed, getTime(data + 1), setTime(data + 1, value));
        EVENT_PROPERTY(uint16_t, time_total, getTime(data + 3), setTime(data + 3, value));

    private:
        uint16_t getTime(const uint8_t* data) const {
            uint16_t value = 0;
            memcpy(&value, data, 2);
            return value;
        }

        void setTime(uint8_t* data, uint16_t value) {
            memcpy(data, &value, 2);
        }
};

class AudioSettingsMenuState : public Event {
    public:
        AudioSettingsMenuState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SETTINGS_STATE_MENU,
                    {0x00, 0x00}) {}

        EVENT_PROPERTY(uint8_t, page, data[0], data[0] = value);
        EVENT_PROPERTY(uint8_t, item, data[1], data[1] = value);
        EVENT_PROPERTY(uint8_t, count, data[2], data[2] = value);
};

class AudioSettingsItemState : public Event {
    public:
        AudioSettingsItemState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SETTINGS_STATE_ITEM,
                    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}) {}

        EVENT_PROPERTY(uint8_t, item, data[1], data[1] = value);
        EVENT_PROPERTY(AudioSettingsType, type,
                (AudioSettingsType)data[2],
                data[2] = (uint8_t)value);
        EVENT_PROPERTY(uint32_t, checksum,
                Endian::nbtohl(data + 2),
                Endian::hltonb(data + 2, value));
};

class AudioSettingsExitState : public Event {
    public:
        AudioSettingsExitState() :
                Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SETTINGS_STATE_ITEM) {}
};

class AudioSettingsOpenCmd : public Event {
    public:
        AudioSettingsOpenCmd() :
                Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SETTINGS_CMD_OPEN) {}
};

class AudioSettingsSelectCmd : public Event {
    public:
        AudioSettingsSelectCmd(uint8_t item = 0x00) :
                Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SETTINGS_CMD_SELECT, {item}) {}

        EVENT_PROPERTY(uint8_t, item, data[0], data[0] = value);
};

class AudioSettingsBackCmd : public Event {
    AudioSettingsBackCmd() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SETTINGS_CMD_BACK) {}
};

class AudioSettingsExitCmd : public Event {
    AudioSettingsExitCmd() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SETTINGS_CMD_EXIT) {}
};

class AudioChecksumEvent : public Event {
    public:
        AudioChecksumEvent(AudioEvent id) : Event(SubSystem::AUDIO, (uint8_t)id) {}

        EVENT_PROPERTY(uint32_t, checksum,
                Endian::nbtohl(data),
                Endian::hltonb(data, value));
};

class AudioTrackTitleState : public AudioChecksumEvent {
    public:
        AudioTrackTitleState() :  AudioChecksumEvent(AudioEvent::TRACK_TITLE_STATE) {}
};

class AudioTrackArtistState : public AudioChecksumEvent {
    public:
        AudioTrackArtistState() :  AudioChecksumEvent(AudioEvent::TRACK_ARTIST_STATE) {}
};

class AudioTrackAlbumState : public AudioChecksumEvent {
    public:
        AudioTrackAlbumState() :  AudioChecksumEvent(AudioEvent::TRACK_ALBUM_STATE) {}
};

// Node for interacting with Garming Fusion head units over J1939/NMEA2000.
class Fusion : public Caster::Node<Message> {
    public:
        // Construct a fusion node. Events with string contents write their
        // payloads to scratch.
        Fusion(Scratch* scratch, Faker::Clock* clock = Faker::Clock::real());

        // Handle J1939 state messages from the head unit and control Events
        // from other devices.
        void handle(const Message& msg, const Caster::Yield<Message>& yield) override;

        // Emit state events from the head units.
        void emit(const Caster::Yield<Message>& yield) override;

    private:
        void handleEvent(const Event& event, const Caster::Yield<Message>& yield);
        void handleSettingsOpenEvent(const Caster::Yield<Message>& yield);
        void handleSettingsSelectEvent(const AudioSettingsSelectCmd* event,
                const Caster::Yield<Message>& yield);
        void handleSettingsBackEvent(const Caster::Yield<Message>& yield);
        void handleSettingsExitEvent(const Caster::Yield<Message>& yield);

        void handleJ1939Claim(const J1939Claim& claim,
                const Caster::Yield<Message>& yield);
        void handleJ1939Message(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleAnnounce(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handlePower(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleHeartbeat(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleSource(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleTrackPlayback(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleTrackTitle(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleTrackArtiat(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleTrackAlbum(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleTimeElapsed(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleRadioFrequency(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleInputGain(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleTone(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleMute(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleBalance(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleVolume(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleMenuLoad(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleMenuItemCount(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleMenuItemList(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleBluetoothConnection(bool connected,
                const Caster::Yield<Message>& yield);

        bool handleString(const Canny::J1939Message& msg, uint8_t offset);

        void resetControlCounter();

        void sendStereoRequest(const Caster::Yield<Message>& yield);
        void sendStereoDiscovery(const Caster::Yield<Message>& yield);
        void sendPower(bool power, const Caster::Yield<Message>& yield);
        void sendSetSource(AudioSource source, const Caster::Yield<Message>& yield);

        void sendMenu(uint8_t page, uint8_t item, const Caster::Yield<Message>& yield);
        void sendMenuSettings(const Caster::Yield<Message>& yield);
        void sendMenuSelectItem(uint8_t item, const Caster::Yield<Message>& yield);
        void sendMenuBack(const Caster::Yield<Message>& yield);
        void sendMenuExit(const Caster::Yield<Message>& yield);
        void sendMenuReqItemCount(const Caster::Yield<Message>& yield);
        void sendMenuReqItemList(uint8_t count, const Caster::Yield<Message>& yield);

        Faker::Clock* clock_;
        Scratch* scratch_;
        CRC32::Checksum checksum_;

        uint8_t address_;
        uint8_t hu_address_;
        Ticker hb_timer_;
        Ticker disco_timer_;

        AudioSystemState system_;
        AudioVolumeState volume_;
        AudioMuteState mute_;
        AudioToneState tone_;
        AudioTrackPlaybackState track_playback_;
        AudioTrackTitleState track_title_;
        AudioTrackArtistState track_artist_;
        AudioTrackAlbumState track_album_;
        AudioSettingsMenuState settings_menu_;
        AudioSettingsItemState settings_item_;
        AudioSettingsExitState settings_exit_;

        bool recent_mute_;

        uint8_t state_;
        uint8_t handle_counter_;
        uint8_t control_counter_;
};

}  // namespace R51

#endif  // _R51_CONTROLLER_FUSION_H_
