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

enum class AudioSeek : uint8_t {
    AUTO = 0x00,
    MANUAL = 0x01,
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
                                // or mute/unmute for radio, aux, or optical
                                // sources.
    PLAYBACK_NEXT_CMD   = 0xE1, // Advance to next track for Bluetooth or auto
                                // seek to next station for radio.
    PLAYBACK_PREV_CMD   = 0xE2, // Go to previous track for Bluetooth or auto
                                // seek to next station for radio.

    // Settings menu events and controls.
    SETTINGS_OPEN_CMD   = 0xF0, // Sent by the user to open the settings menu.
    SETTINGS_BACK_CMD   = 0xF1, // Sent by the user to go the the previous settings menu.
    SETTINGS_EXIT_CMD   = 0xF2, // Sent by the user to exit the settings menu.
    SETTINGS_SELECT_CMD = 0xF3, // Sent by the user to select a settings menu item.
    SETTINGS_MENU_STATE = 0xF4, // Sent by Fusion to display a settings menu.
    SETTINGS_ITEM_STATE = 0xF5, // Sent by Fusion to display an item in a settings menu.
    SETTINGS_EXIT_STATE = 0xF6, // Sent by Fusion to stop displaying the settings menu.
};

class AudioSystemState : public Event {
    public:
        AudioSystemState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SYSTEM_STATE,
                    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}) {}

        EVENT_PROPERTY(bool, available, getBit(data, 0, 4), setBit(data, 0, 4, value));
        EVENT_PROPERTY(bool, power, getBit(data, 0, 5), setBit(data, 0, 5, value));
        EVENT_PROPERTY(bool, bt_connected, getBit(data, 0, 6), setBit(data, 0, 6, value));
        EVENT_PROPERTY(AudioSeek, seek_mode,
                (AudioSeek)getBit(data, 0, 7),
                setBit(data, 0, 7, (bool)value));
        EVENT_PROPERTY(AudioSource, source,
                (AudioSource)(data[0] & 0x0F),
                data[0] = (data[0] & 0xF0) | ((uint8_t)value & 0x0F));
        EVENT_PROPERTY(int8_t, gain, (int8_t)data[1], data[1] = (uint8_t)value);
        EVENT_PROPERTY(uint32_t, frequency,
                Endian::nbtohl(data + 2),
                Endian::hltonb(data + 2, value));

        void toggle_seek_mode() {
            flipBit(data, 0, 7);
        }
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
        EVENT_PROPERTY(uint32_t, checksum,
                Endian::nbtohl(data + 2),
                Endian::hltonb(data + 2, value));
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
                Endian::nbtohl(data),
                Endian::hltonb(data, value));
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

        void handleSourceNextCmd(const Caster::Yield<Message>& yield);
        void handleSourcePrevCmd(const Caster::Yield<Message>& yield);

        void handlePlaybackToggleCmd(const Caster::Yield<Message>& yield);
        void handlePlaybackNextCmd(const Caster::Yield<Message>& yield);
        void handlePlaybackPrevCmd(const Caster::Yield<Message>& yield);

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
