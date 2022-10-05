#ifndef _R51_CONTROLLER_FUSION_H_
#define _R51_CONTROLLER_FUSION_H_

#include <Arduino.h>
#include <CRC32.h>
#include <Caster.h>
#include <Common.h>
#include <Endian.h>
#include <Faker.h>
#include <Foundation.h>

namespace R51 {

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

enum class AudioPlayback : uint8_t {
    NO_TRACK = 0x00,
    PLAY = 0x01,
    PAUSE = 0x02,
};

enum class AudioEvent {
    // State events.
    SYSTEM_STATE            = 0x00, // State event. Sends power, connectivity, gain, frequency.
    VOLUME_STATE            = 0x01, // State event. Sends current volume, fade, and balance.
    TONE_STATE              = 0x02, // State event. Sends equalizer high/mid/low values.
    TRACK_PLAYBACK_STATE    = 0x03, // State event. Sends the playback status and track times.
    TRACK_TITLE_STATE       = 0x04, // State event. Sends the track title.
    TRACK_ARTIST_STATE      = 0x05, // State event. Sends the artist name.
    TRACK_ALBUM_STATE       = 0x06, // State event. Sends the album title.

    // System controls.
    POWER_ON                = 0x10, // Turn on the stereo.
    POWER_OFF               = 0x11, // Turn off the stereo.
    SOURCE_SET              = 0x12, // Set source.

    // Track controls.
    TRACK_PLAY              = 0x20, // Play current track.
    TRACK_PAUSE             = 0x21, // Pause current track.
    TRACK_NEXT              = 0x22, // Next track.
    TRACK_PREV              = 0x23, // Prev track.

    // Radio controls.
    RADIO_TUNE              = 0x30, // Tune radio to a specific frequency.
    RADIO_NEXT_AUTO         = 0x31, // Next radio frequency in auto seek mode.
    RADIO_PREV_AUTO         = 0x32, // Prev radio frequency in auto seek mode.
    RADIO_NEXT_MANUAL       = 0x33, // Next radio frequency in manual seek mode.
    RADIO_PREV_MANUAL       = 0x34, // Prev radio frequency in manual seek mode.

    // Input controls.
    INPUT_GAIN_SET          = 0x40, // Set the input gain to specific value.
    INPUT_GAIN_INC          = 0x41, // Increment the input gain value.
    INPUT_GAIN_DEC          = 0x42, // Decrement the input gain value.

    // Volums controls.
    VOLUME_SET              = 0x50, // Set volume.
    VOLUME_INC              = 0x51, // Increment volume.
    VOLUME_DEC              = 0x52, // Decrement volume.
    VOLUME_MUTE             = 0x53, // Mute volume.
    VOLUME_UNMUTE           = 0x54, // Unmute volume.
    BALANCE_SET             = 0x55, // Set the audio balance.
    BALANCE_LEFT            = 0x56, // Shift audio balance to the left.
    BALANCE_RIGHT           = 0x57, // Shift audio balance to the right.
    FADE_SET                = 0x58, // Set the audio fade.
    FADE_FRONT              = 0x59, // Fade audio to the front.
    FADE_REAR               = 0x5A, // Fade audio to the back.

    // EQ controls.
    TONE_SET                = 0x60, // Set bass, mid, and treble.
    TONE_BASS_INC           = 0x61, // Increase bass.
    TONE_BASS_DEC           = 0x62, // Decrease bass.
    TONE_MID_INC            = 0x63, // Increase mid.
    TONE_MID_DEC            = 0x64, // Decrease mid.
    TONE_TREBLE_INC         = 0x65, // Increase treble.
    TONE_TREBLE_DEC         = 0x66, // Decrease treble.

    // Settings menu events and controls.
    SETTINGS_CMD_OPEN       = 0xF0, // Sent by the user to open the settings menu.
    SETTINGS_CMD_BACK       = 0xF1, // Sent by the user to go the the previous settings menu.
    SETTINGS_CMD_EXIT       = 0xF2, // Sent by the user to exit the settings menu.
    SETTINGS_CMD_SELECT     = 0xF3, // Sent by the user to select a settings menu item.
    SETTINGS_STATE_MENU     = 0xF4, // Sent by Fusion to display a settings menu.
    SETTINGS_STATE_ITEM     = 0xF5, // Sent by Fusion to display an item in a settings menu.
    SETTINGS_STATE_EXIT     = 0xF6, // Sent by Fusion to stop displaying the settings menu.
};

class AudioSystemState : public Event {
    public:
        AudioSystemState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SYSTEM_STATE,
                    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}) {}

        EVENT_PROPERTY(bool, available, getBit(data, 0, 4), setBit(data, 0, 4, value));
        EVENT_PROPERTY(bool, power, getBit(data, 0, 5), setBit(data, 0, 5, value));
        EVENT_PROPERTY(bool, bt_connected, getBit(data, 0, 6), setBit(data, 0, 6, value));
        EVENT_PROPERTY(AudioSource, source,
                (AudioSource)(data[0] & 0x0F),
                data[0] = (data[0] & 0xF0) | ((uint8_t)value & 0x0F));
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
        EVENT_PROPERTY(bool, mute, data[3] != 0, data[3] = (uint8_t)value);
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

enum class AudioSettingsType : uint8_t {
    SUBMENU = 1,
    SELECT = 2,
    CHECKBOX_OFF = 3,
    CHECKBOX_ON = 4,
};

class AudioSettingsItemState : public Event {
    public:
        AudioSettingsItemState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SETTINGS_STATE_ITEM,
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

class AudioSourceSetEvent : public Event {
    public:
        AudioSourceSetEvent() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SOURCE_SET, {0x00}) {}

        EVENT_PROPERTY(AudioSource, source,
                (AudioSource)data[0],
                data[0] = (uint8_t)value);
};

class AudioRadioTuneEvent : public Event {
    public:
        AudioRadioTuneEvent() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::RADIO_TUNE,
                    {0x00, 0x00, 0x00, 0x00}) {}

        EVENT_PROPERTY(uint32_t, frequency,
                Endian::nbtohl(data),
                Endian::hltonb(data, value));
};

class AudioInputGainSetEvent : public Event {
    public:
        AudioInputGainSetEvent() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::INPUT_GAIN_SET, {0x00}) {}

        EVENT_PROPERTY(int8_t, gain, (int8_t)data[0], data[0] = (uint8_t)value);
};

class AudioVolumeSetEvent : public Event {
    public:
        AudioVolumeSetEvent() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::VOLUME_SET, {0x00}) {}

        EVENT_PROPERTY(uint8_t, volume, data[0], data[0] = value);
};

class AudioBalanceSetEvent : public Event {
    public:
        AudioBalanceSetEvent() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::BALANCE_SET, {0x00}) {}

        EVENT_PROPERTY(int8_t, balance, (int8_t)data[0], data[0] = (uint8_t)value);
};

class AudioFadeSetEvent : public Event {
    public:
        AudioFadeSetEvent() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::BALANCE_SET, {0x00}) {}

        EVENT_PROPERTY(int8_t, fade, (int8_t)data[0], data[0] = (uint8_t)value);
};

class AudioToneSetEvent : public Event {
    public:
        AudioToneSetEvent() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::TONE_SET,
                    {0x00, 0x00, 0x00}) {}

        EVENT_PROPERTY(int8_t, bass, (int8_t)data[0], data[0] = (uint8_t)value);
        EVENT_PROPERTY(int8_t, mid, (int8_t)data[1], data[1] = (uint8_t)value);
        EVENT_PROPERTY(int8_t, treble, (int8_t)data[2], data[2] = (uint8_t)value);
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

        void handleJ1939Claim(const J1939Claim& claim,
                const Caster::Yield<Message>& yield);
        void handleJ1939Message(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);

        void handleState(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleAnnounce(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handlePower(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleHeartbeat(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleSource(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleTrackPlayback(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleTrackString(uint8_t seq, const Canny::J1939Message& msg,
                AudioChecksumEvent* event, const Caster::Yield<Message>& yield);
        void handleTimeElapsed(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleRadioFrequency(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleInputGain(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleTone(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleMute(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleBalance(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleVolume(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleMenuLoad(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleMenuItemCount(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleMenuItemList(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleBluetoothConnection(bool connected,
                const Caster::Yield<Message>& yield);

        bool handleString(uint8_t seq, const Canny::J1939Message& msg, uint8_t offset);

        void sendStereoRequest(const Caster::Yield<Message>& yield);
        void sendStereoDiscovery(const Caster::Yield<Message>& yield);

        void sendCmd(const Caster::Yield<Message>& yield,
                uint8_t cs, uint8_t id, uint8_t payload0 = 0xFF, uint8_t payload1 = 0xFF);
        void sendCmdPayload(const Caster::Yield<Message>& yield, uint32_t payload);
        template <size_t N> 
        void sendCmdPayload(const Caster::Yield<Message>& yield, const uint8_t (&payload)[N]);

        void sendPowerCmd(const Caster::Yield<Message>& yield, bool power);
        void sendSourceSetCmd(const Caster::Yield<Message>& yield, AudioSource source);
        void sendTrackCmd(const Caster::Yield<Message>& yield, uint8_t cmd);
        void sendRadioCmd(const Caster::Yield<Message>& yield, uint8_t cmd, uint32_t freq);
        void sendInputGainSetCmd(const Caster::Yield<Message>& yield, int8_t gain);
        void sendVolumeSetCmd(const Caster::Yield<Message>& yield, uint8_t volume, int8_t fade);
        void sendVolumeMuteCmd(const Caster::Yield<Message>& yield, bool mute);
        void sendBalanceSetCmd(const Caster::Yield<Message>& yield, int8_t balance);
        void sendToneSetCmd(const Caster::Yield<Message>& yield,
                int8_t bass, int8_t treble, int8_t mid);

        void sendMenu(const Caster::Yield<Message>& yield, uint8_t page, uint8_t item);
        void sendMenuSettings(const Caster::Yield<Message>& yield);
        void sendMenuSelectItem(const Caster::Yield<Message>& yield, uint8_t item);
        void sendMenuBack(const Caster::Yield<Message>& yield);
        void sendMenuExit(const Caster::Yield<Message>& yield);
        void sendMenuReqItemCount(const Caster::Yield<Message>& yield);
        void sendMenuReqItemList(const Caster::Yield<Message>& yield, uint8_t count);

        uint8_t volume();

        Faker::Clock* clock_;
        Scratch* scratch_;
        CRC32::Checksum checksum_;

        uint8_t address_;
        uint8_t hu_address_;
        Ticker hb_timer_;
        Ticker disco_timer_;

        AudioSystemState system_;
        AudioVolumeState volume_;
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
        bool state_ignore_;
        uint8_t state_counter_;
        uint8_t cmd_counter_;
        Canny::J1939Message cmd_;

        AudioSource secondary_source_;
};

template <size_t N> 
void Fusion::sendCmdPayload(const Caster::Yield<Message>& yield, const uint8_t (&payload)[N]) {
    uint8_t i;
    cmd_.data()[0] = cmd_counter_++;
    for (i = 0; i < N; ++i) {
        cmd_.data()[i + 1] = payload[i];
    }
    memset(cmd_.data() + N + 1, 0xFF, 7 - N);
    yield(cmd_);
}

}  // namespace R51

#endif  // _R51_CONTROLLER_FUSION_H_
