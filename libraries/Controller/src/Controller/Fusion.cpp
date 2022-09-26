#include "Fusion.h"

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>

namespace R51 {
namespace {

using ::Canny::J1939Message;

enum State : uint8_t {
    SOURCE = 0x02,
    PLAYBACK = 0x04,
    TRACK = 0x05,
    ARTIST = 0x06,
    ALBUM = 0x07,
    TIME_ELAPSED = 0x09,
    FREQUENCY = 0x0B,
    GAIN = 0x13,
    TONE = 0x16,
    MUTE = 0x17,
    BALANCE = 0x18,
    VOLUME = 0x1D,
};

uint8_t counter(const J1939Message& msg) {
    return msg.data()[0];
}

uint8_t id(uint8_t counter) {
    return (counter & 0xE0) >> 4;
}

uint8_t id(const J1939Message& msg) {
    return id(counter(msg));
}

uint8_t seq(uint8_t counter) {
    return counter & 0x1F;
}

uint8_t seq(const J1939Message& msg)  {
    return seq(counter(msg));
}

State detectState(const J1939Message& msg) {
    if (msg.data()[2] != 0xA3 || msg.data()[3] != 0x99 || msg.data()[5] != 0x80) {
        return (State)0xFF;
    }
    return (State)msg.data()[4];
}

}  // namespace

Fusion::Fusion(uint8_t address, uint8_t hu_address, Scratch* scratch) :
        address_(address), hu_address_(hu_address), scratch_(scratch),
        emit_volume_(false), emit_mute_(false), emit_eq_(false),
        emit_source_(false), emit_playback_(false), emit_track_(false),
        emit_artist_(false), emit_album_(false), recent_mute_(false),
        state_(0xFF), handle_counter_(0xFF) {}

void Fusion::handle(const Message& msg, const Caster::Yield<Message>&) {
    switch (msg.type()) {
        case Message::EVENT:
            if (msg.event().subsystem == (uint8_t)SubSystem::AUDIO) {
                handleEvent(msg.event());
            }
            break;
        case Message::J1939_MESSAGE:
            if (msg.j1939_message().source_address() == hu_address_) {
                handleJ1939(msg.j1939_message());
            }
            break;
        default:
            break;
    }
}

void Fusion::handleEvent(const Event& event) {
    // TODO implement
}

void Fusion::handleJ1939(const J1939Message& msg) {
    if (id(msg) != id(handle_counter_)) {
        state_ = detectState(msg);
    }
    handle_counter_ = counter(msg);

    switch (state_) {
        case SOURCE:
            handleSource(msg);
            break;
        case PLAYBACK:
            handlePlayback(msg);
            break;
        case TRACK:
            handleTrack(msg);
            break;
        case ARTIST:
            handleArtist(msg);
            break;
        case ALBUM:
            handleAlbum(msg);
            break;
        case TIME_ELAPSED:
            handleTimeElapsed(msg);
            break;
        case FREQUENCY:
            handleFrequency(msg);
            break;
        case GAIN:
            handleGain(msg);
            break;
        case TONE:
            handleTone(msg);
            break;
        case MUTE:
            handleMute(msg);
            break;
        case BALANCE:
            handleBalance(msg);
            break;
        case VOLUME:
            handleVolume(msg);
            break;
        default:
            break;
    }
}

void Fusion::handleSource(const J1939Message& msg) {
    if (seq(msg) != 0 || msg.data()[6] != msg.data()[7]) {
        return;
    }
    emit_source_ |= source_.source((AudioSource)msg.data()[7]);
}

void Fusion::handlePlayback(const J1939Message& msg) {
    uint32_t time;
    switch (seq(msg)) {
        case 0:
            if (msg.data()[7] > 0x02) {
                msg.data()[7] = 0x00;
            }
            emit_playback_ |= playback_.playback((AudioPlayback)msg.data()[7]);
            break;
        case 2:
            time = msg.data()[3];
            time |= (msg.data()[4] << 8);
            time |= (msg.data()[5] << 16);
            emit_playback_ |= playback_.time_total(time / 1000);
            break;
        default:
            break;
    }
}

void Fusion::handleTrack(const J1939Message& msg) {
    if (handleString(msg)) {
        emit_track_ |= track_.checksum(checksum_.value());
    }
}

void Fusion::handleArtist(const J1939Message& msg) {
    if (handleString(msg)) {
        emit_artist_ |= artist_.checksum(checksum_.value());
    }
}

void Fusion::handleAlbum(const J1939Message& msg) {
    if (handleString(msg)) {
        emit_album_ |= album_.checksum(checksum_.value());
    }
}

void Fusion::handleTimeElapsed(const J1939Message& msg) {
    if (seq(msg) == 1) {
        uint32_t time = msg.data()[1];
        time |= (msg.data()[2] << 8);
        time |= (msg.data()[3] << 16);
        emit_playback_ |= playback_.time_elapsed(time / 4);
    }
}

void Fusion::handleFrequency(const J1939Message& msg) {
    if (seq(msg) == 1) {
        uint32_t frequency = msg.data()[1];
        frequency |= (msg.data()[2] << 8);
        frequency |= (msg.data()[3] << 16);
        frequency |= (msg.data()[4] << 24);
        emit_source_ |= source_.frequency(frequency);
    }
}

void Fusion::handleGain(const J1939Message& msg) {
    if (seq(msg) == 0) {
        emit_source_ |= source_.gain((int8_t)msg.data()[7]);
    }
}

void Fusion::handleTone(const J1939Message& msg) {
    // We set all zones to the same EQ so we only care about reading the first
    // zone.
    if (msg.data()[6] != 0x00) {
        return;
    }
    switch (seq(msg)) {
        case 0:
            emit_eq_ |= eq_.bass(msg.data()[7]);
            break;
        case 1:
            emit_eq_ |= eq_.mid(msg.data()[1]);
            emit_eq_ |= eq_.treble(msg.data()[2]);
            break;
        default:
            break;
    }
}

void Fusion::handleMute(const J1939Message& msg) {
    if (seq(msg) == 0) {
        emit_mute_ |= mute_.mute(msg.data()[6] != 0x00);
    }
}

void Fusion::handleBalance(const J1939Message& msg) {
    // We balance all zones together so we only need to read
    // balance from zone 1.
    if (seq(msg) == 0 && msg.data()[6] == 0x00) {
        emit_volume_ |= volume_.balance(msg.data()[7]);
    }
}

void Fusion::handleVolume(const J1939Message& msg) {
    // The second message contains the volum for zone 3. We don't use zone 3
    // because we're mimicing a car stereo with front/rear. So we only parse
    // the first message.
    if (seq(msg) == 0) {
        uint8_t zone1 = msg.data()[6];
        uint8_t zone2 = msg.data()[7];
        if (zone1 > zone2) {
            emit_volume_ |= volume_.volume(zone1);
        } else {
            emit_volume_ |= volume_.volume(zone2);
        }
        emit_volume_ |= volume_.fade(zone1 - zone2);
        if (recent_mute_) {
            emit_volume_ = false;
            recent_mute_ = false;
        }
    }
}

bool Fusion::handleString(const J1939Message& msg) {
    if (seq(msg) == 0) {
        scratch_->size = 0;
        checksum_.reset();
        return false;
    }

    uint8_t i = 0;
    if (seq(msg) == 1) {
        // skip prefix in first frame
        i = 4;
    }
    for (; i < 7; i++) {
        if (scratch_->size >= 256) {
            // buffer overflow
            return false;
        }

        scratch_->bytes[scratch_->size] = msg.data()[i + 1];
        checksum_.update(msg.data()[i + 1]);
        if (scratch_->bytes[scratch_->size] == 0) {
            // end of string
            return true;
        }
        ++(scratch_->size);
    }
    return false;
}

void Fusion::emit(const Caster::Yield<Message>& yield) {
    if (emit_volume_) {
        emit_volume_ = false;
        yield(volume_);
    }
    if (emit_eq_) {
        emit_eq_ = false;
        yield(eq_);
    }
    if (emit_source_) {
        emit_source_ = false;
        yield(source_);
    }
    if (emit_playback_) {
        emit_playback_ = false;
        yield(playback_);
    }
    if (emit_track_) {
        emit_track_ = false;
        yield(track_);
    }
    if (emit_artist_) {
        emit_artist_ = false;
        yield(artist_);
    }
    if (emit_album_) {
        emit_album_ = false;
        yield(album_);
    }
}

}  // namespace R51
