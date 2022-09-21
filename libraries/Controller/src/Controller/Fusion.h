#ifndef _R51_CONTROLLER_FUSION_H_
#define _R51_CONTROLLER_FUSION_H_

#include <Arduino.h>
#include <CRC32.h>
#include <Caster.h>
#include <Common.h>

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

enum class Playback : uint8_t {
    NO_TRACK = 0x00,
    PLAY = 0x01,
    PAUSE = 0x02,
};

enum class AudioEvent {
    VOLUME_STATE = 0x01,        // State event. Sends current volume, fade, and balance.
    EQ_STATE = 0x02,            // State event. Sends equalizer high/mid/low values.
    SOURCE_STATE = 0x03,        // State event. Sends the current audio source, gain,
                                // and frequency when applicable.
    PLAYBACK_STATE = 0x04,      // State event. Sends the playback status and track times.
    TRACK_STATE = 0x05,         // State event. Sends the track title.
    ARTIST_STATE = 0x06,        // State event. Sends the artist name.
    ALBUM_STATE = 0x07,         // State event. Sends the album title.

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

class AudioVolumeState : public Event {
    public:
        AudioVolumeState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::VOLUME_STATE,
                    {0x00, 0x00, 0x00, 0x00}) {}

        EVENT_PROPERTY(uint8_t, volume, data[0], data[0] = value);
        EVENT_PROPERTY(int8_t, fade, (int8_t)data[1], data[1] = (uint8_t)value);
        EVENT_PROPERTY(int8_t, balance, (int8_t)data[2], data[2] = (uint8_t)value);
        EVENT_PROPERTY(bool, mute, data[3] == 1, data[3] = (uint8_t)value);
};

class  AudioEqState : public Event {
    public:
        AudioEqState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::EQ_STATE,
                    {0x00, 0x00, 0x00}) {}

        EVENT_PROPERTY(int8_t, bass, (int8_t)data[0], data[0] = (uint8_t)value);
        EVENT_PROPERTY(int8_t, mid, (int8_t)data[1], data[1] = (uint8_t)value);
        EVENT_PROPERTY(int8_t, treble, (int8_t)data[2], data[3] = (uint8_t)value);
};

class AudioSourceState : public Event {
    public:
        AudioSourceState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::SOURCE_STATE,
                    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}) {}

        EVENT_PROPERTY(uint8_t, source, data[0], data[0] = value);
        EVENT_PROPERTY(int8_t, gain, (int8_t)data[1], data[1] = (uint8_t)value);
        EVENT_PROPERTY(uint32_t, frequency, getFrequency(), setFrequency(value));

    private:
        uint32_t getFrequency() const {
            uint32_t value = 0;
            memcpy(&value, data + 2, 4);
            return value;
        }

        void setFrequency(uint32_t value) {
            memcpy(data + 2, &value, 4);
        }
};

class AudioPlaybackState : public Event {
    public:
        AudioPlaybackState() :
            Event(SubSystem::AUDIO, (uint8_t)AudioEvent::PLAYBACK_STATE,
                    {0x00, 0x00, 0x00, 0x00, 0x00}) {}

        EVENT_PROPERTY(Playback, playback, (Playback)data[0], data[0] = (uint8_t)value);
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

class AudioTrackState : public AudioChecksumEvent {
    public:
        AudioTrackState() :  AudioChecksumEvent(AudioEvent::TRACK_STATE) {}
};

class AudioArtistState : public AudioChecksumEvent {
    public:
        AudioArtistState() :  AudioChecksumEvent(AudioEvent::ARTIST_STATE) {}
};

class AudioAlbumState : public AudioChecksumEvent {
    public:
        AudioAlbumState() :  AudioChecksumEvent(AudioEvent::ALBUM_STATE) {}
};

// Node for interacting with Garming Fusion head units over J1939/NMEA2000.
class Fusion : public Caster::Node<Message> {
    public:
        // Construct a fusion node which communicates from the provided
        // address. Events with string contents write their payloads to
        // scratch.
        Fusion(uint8_t address, uint8_t hu_address, Scratch* scratch);

        // Handle J1939 state messages from the head unit and control Events
        // from other devices.
        void handle(const Message& msg) override;

        // Emit state events from the head units.
        void emit(const Caster::Yield<Message>& yield) override;

    private:
        void handleEvent(const Event& event);
        void handleJ1939(const Canny::J1939Message& msg);

        void handleSource(const Canny::J1939Message& msg);
        void handlePlayback(const Canny::J1939Message& msg);
        void handleTrack(const Canny::J1939Message& msg);
        void handleArtist(const Canny::J1939Message& msg);
        void handleAlbum(const Canny::J1939Message& msg);
        void handleTimeElapsed(const Canny::J1939Message& msg);
        void handleFrequency(const Canny::J1939Message& msg);
        void handleGain(const Canny::J1939Message& msg);
        void handleTone(const Canny::J1939Message& msg);
        void handleMute(const Canny::J1939Message& msg);
        void handleBalance(const Canny::J1939Message& msg);
        void handleVolume(const Canny::J1939Message& msg);

        bool handleString(const Canny::J1939Message& msg);

        uint8_t address_;
        uint8_t hu_address_;
        Scratch* scratch_;
        CRC32::Checksum checksum_;

        AudioVolumeState volume_;
        AudioEqState eq_;
        AudioSourceState source_;
        AudioPlaybackState playback_;
        AudioTrackState track_;
        AudioArtistState artist_;
        AudioAlbumState album_;

        bool emit_volume_;
        bool emit_eq_;
        bool emit_source_;
        bool emit_playback_;
        bool emit_track_;
        bool emit_artist_;
        bool emit_album_;

        uint8_t state_;
        uint8_t counter_;
};

}  // namespace R51

#endif  // _R51_CONTROLLER_FUSION_H_
