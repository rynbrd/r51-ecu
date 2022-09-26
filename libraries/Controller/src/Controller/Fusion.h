#ifndef _R51_CONTROLLER_FUSION_H_
#define _R51_CONTROLLER_FUSION_H_

#include <Arduino.h>
#include <CRC32.h>
#include <Caster.h>
#include <Common.h>
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
    SYSTEM_STATE = 0x00,        // State event. Sends power, connectivity, gain, frequency.
    VOLUME_STATE = 0x01,        // State event. Sends current volume, fade, and balance.
    MUTE_STATE = 0x02,          // State event. Sent on mute/unmute.
    EQ_STATE = 0x03,            // State event. Sends equalizer high/mid/low values.
    TRACK_PLAYBACK_STATE = 0x04,      // State event. Sends the playback status and track times.
    TRACK_TITLE_STATE = 0x05,         // State event. Sends the track title.
    TRACK_ARTIST_STATE = 0x06,        // State event. Sends the artist name.
    TRACK_ALBUM_STATE = 0x07,         // State event. Sends the album title.

    NEXT = 0x11,                // Next track/frequency/gain value.
    PREV = 0x12,                // Prev track/frequency/gain value.
    INC_VOLUME = 0x13,          // Increment volume.
    DEC_VOLUME = 0x14,          // Decrement volume.
    BALANCE_LEFT = 0x15,        // Shift audio balance to the left.
    BALANCE_RIGHT = 0x16,       // Shift audio balance to the right.
    FADE_FRONT = 0x17,          // Fade audio to the front.
    FADE_BACK = 0x18,           // Fade audio to the back.
    INC_BASS_TONE = 0x19,       // Increase bass.
    DEC_BASS_TONE = 0x1A,       // Decrease bass.
    INC_MID_TONE = 0x1B,        // Increase mid.
    DEC_MID_TONE = 0x1C,        // Decrease mid.
    INC_TREBLE_TONE = 0x1D,     // Increase treble.
    DEC_TREBLE_TONE = 0x1E,     // Decrease treble.
    SET_SOURCE = 0x1F,          // Set source.
    TOGGLE_PLAYBACK = 0x20,     // Toggle play/pause.
    TOGGLE_MUTE = 0x21,         // Toggle mute/unmute.
    TOGGLE_SCAN_MODE = 0x22,    // Toggle radio scan mode.
    TOGGLE_BT_DISCO = 0x23,     // Toggle Bluetooth discovery on/off.
    CONNECT_BT = 0x24,          // Connect to a Bluetooth device.
    FORGET_BT = 0x25,           // Forget a Bluetooth device.
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
        EVENT_PROPERTY(uint32_t, frequency, getFrequency(), setFrequency(value));

    private:
        uint32_t getFrequency() const {
            uint32_t value = 0;
            NetworkToUInt32(&value, data + 2);
            return value;
        }

        void setFrequency(uint32_t value) {
            UInt32ToNetwork(data + 2, value);
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

class AudioChecksumEvent : public Event {
    public:
        AudioChecksumEvent(AudioEvent id) : Event(SubSystem::AUDIO, (uint8_t)id) {}

        EVENT_PROPERTY(uint32_t, checksum, getChecksum(), setChecksum(value));

    private:
        uint32_t getChecksum() const {
            uint32_t value = 0;
            memcpy(&value, data, 4);
            return value;
        }

        void setChecksum(uint32_t value) {
            memcpy(data, &value, 4);
        }
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
        void handleEvent(const Event& event,
                const Caster::Yield<Message>& yield);
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
        void handleBluetoothConnection(bool connected,
                const Caster::Yield<Message>& yield);

        bool handleString(const Canny::J1939Message& msg);

        void sendStereoRequest(const Caster::Yield<Message>& yield);
        void sendStereoDiscovery(const Caster::Yield<Message>& yield);

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

        bool recent_mute_;

        uint8_t state_;
        uint8_t handle_counter_;
};

}  // namespace R51

#endif  // _R51_CONTROLLER_FUSION_H_
