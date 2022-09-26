#include "Fusion.h"

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>
#include <Faker.h>

namespace R51 {
namespace {

using ::Canny::J1939Message;
using ::Faker::Clock;

static const uint32_t kAvailabilityTimeout = 5000;
static const uint32_t kDiscoveryTick = 5000;

enum State : uint8_t {
    // 4th byte of A3:99:XX:80 state frames.
    SOURCE = 0x02,
    TRACK_PLAYBACK = 0x04,
    TRACK_TITLE = 0x05,
    TRACK_ARTIST = 0x06,
    TRACK_ALBUM = 0x07,
    TRACK_ELAPSED = 0x09,
    RADIO_FREQUENCY = 0x0B,
    INPUT_GAIN = 0x13,
    TONE = 0x16,
    MUTE = 0x17,
    BALANCE = 0x18,
    VOLUME = 0x1D,
    HEARTBEAT = 0x20,
    POWER = 0x39,

    // Other state frames.
    INFO14 = 0xF0,
    INFO16 = 0xF1,
    BLUETOOTH_CONNECT = 0xF2,
    BLUETOOTH_DISCONNECT = 0xF3,
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

template <size_t N> 
bool match(const uint8_t* data, const uint8_t (&match)[N]) {
    for (uint8_t i = 0; i < N; i++) {
        if (match[i] != data[i] && match[i] != 0xFF) {
            return false;
        }
    } 
    return true;
}

State detectState(const J1939Message& msg, uint8_t hu_address) {
    if (msg.pgn() == 0x1F014) {
        return State::INFO14;
    } else if (msg.pgn() == 0x1F016) {
        return State::INFO16;
    } else if (msg.pgn() == 0x1FF04 && msg.source_address() == hu_address) {
        if (match(msg.data() + 2, {0xA3, 0x99, 0xFF, 0x80})) {
            return (State)msg.data()[4];
        } else if (match(msg.data() + 2, {0xA5, 0x02, 0x42, 0x54})) {
            return State::BLUETOOTH_CONNECT;
        } else if (match(msg.data() + 2, {0x85, 0x02, 0x42, 0x54})) {
            return State::BLUETOOTH_DISCONNECT;
        }
    }
    return (State)0xFF;
}

}  // namespace

Fusion::Fusion(Scratch* scratch, Clock* clock) :
        clock_(clock), scratch_(scratch),
        address_(Canny::NullAddress), hu_address_(Canny::NullAddress),
        hb_timer_(kAvailabilityTimeout, clock), disco_timer_(kDiscoveryTick, clock),
        recent_mute_(false), state_(0xFF), handle_counter_(0xFF) {}

void Fusion::handle(const Message& msg, const Caster::Yield<Message>& yield) {
    switch (msg.type()) {
        case Message::EVENT:
            if (msg.event().subsystem == (uint8_t)SubSystem::AUDIO) {
                handleEvent(msg.event(), yield);
            }
            break;
        case Message::J1939_CLAIM:
            handleJ1939Claim(msg.j1939_claim(), yield);
            break;
        case Message::J1939_MESSAGE:
            handleJ1939Message(msg.j1939_message(), yield);
            break;
        default:
            break;
    }
}

void Fusion::handleEvent(const Event& event, const Caster::Yield<Message>& yield) {
    // TODO implement
}

void Fusion::handleJ1939Claim(const J1939Claim& claim, const Caster::Yield<Message>& yield) {
    address_ = claim.address();
    if (address_ != Canny::NullAddress && hu_address_ == Canny::NullAddress) {
        sendStereoDiscovery(yield);
    }
}

void Fusion::handleJ1939Message(const J1939Message& msg, const Caster::Yield<Message>& yield) {
    if (address_ == Canny::NullAddress) {
        return;
    }

    if (id(msg) != id(handle_counter_)) {
        state_ = detectState(msg, hu_address_);
    } else {
    }
    handle_counter_ = counter(msg);

    switch (state_) {
        case SOURCE:
            handleSource(msg, yield);
            break;
        case TRACK_PLAYBACK:
            handleTrackPlayback(msg, yield);
            break;
        case TRACK_TITLE:
            handleTrackTitle(msg, yield);
            break;
        case TRACK_ARTIST:
            handleTrackArtiat(msg, yield);
            break;
        case TRACK_ALBUM:
            handleTrackAlbum(msg, yield);
            break;
        case TRACK_ELAPSED:
            handleTimeElapsed(msg, yield);
            break;
        case RADIO_FREQUENCY:
            handleRadioFrequency(msg, yield);
            break;
        case INPUT_GAIN:
            handleInputGain(msg, yield);
            break;
        case TONE:
            handleTone(msg, yield);
            break;
        case MUTE:
            handleMute(msg, yield);
            break;
        case BALANCE:
            handleBalance(msg, yield);
            break;
        case VOLUME:
            handleVolume(msg, yield);
            break;
        case HEARTBEAT:
            handleHeartbeat(msg, yield);
            break;
        case POWER:
            handlePower(msg, yield);
            break;
        case INFO14:
            handleAnnounce(msg, yield);
            break;
        case BLUETOOTH_CONNECT:
            handleBluetoothConnection(true, yield);
            break;
        case BLUETOOTH_DISCONNECT:
            handleBluetoothConnection(false, yield);
            break;
        default:
            break;
    }
}

void Fusion::handleAnnounce(const Canny::J1939Message& msg,
        const Caster::Yield<Message>& yield) {
    // 19F0140A#A0:86:35:08:8E:12:4D:53
    if (seq(msg) != 0 || !match(msg.data() + 1, {0x86, 0x35})) {
        return;
    }
    hu_address_ = msg.source_address();
    Serial.print("found stereo: ");
    if (hu_address_ <= 0x0F) {
        Serial.print("0");
    }
    Serial.println(hu_address_, HEX);
    hb_timer_.reset();
    disco_timer_.reset();
    if (system_.available(true) || system_.power(false)) {
        yield(system_);
    }
    sendStereoRequest(yield);
}

void Fusion::handlePower(const Canny::J1939Message& msg,
        const Caster::Yield<Message>& yield) {
    if (seq(msg) != 0) {
        return;
    }
    if (system_.power(msg.data()[6] != 0x00)) {
        yield(system_);
    }
}

void Fusion::handleHeartbeat(const Canny::J1939Message& msg,
        const Caster::Yield<Message>& yield) {
    if (seq(msg) != 0) {
        return;
    }
    hb_timer_.reset();
    if (system_.power(msg.data()[6] == 0x01)) {
        yield(system_);
    }
}

void Fusion::handleSource(const J1939Message& msg,
        const Caster::Yield<Message>& yield) {
    if (seq(msg) != 0 || msg.data()[6] != msg.data()[7]) {
        return;
    }
    if (system_.source((AudioSource)msg.data()[7])) {
        yield(system_);
    }
}

void Fusion::handleTrackPlayback(const J1939Message& msg,
        const Caster::Yield<Message>& yield) {
    uint32_t time;
    switch (seq(msg)) {
        case 0:
            if (msg.data()[7] > 0x02) {
                msg.data()[7] = 0x00;
            }
            if (track_playback_.playback((AudioPlayback)msg.data()[7])) {
                yield(track_playback_);
            }
            break;
        case 2:
            time = msg.data()[3];
            time |= (msg.data()[4] << 8);
            time |= (msg.data()[5] << 16);
            if (track_playback_.time_total(time / 1000)) {
                yield(track_playback_);
            }
            break;
        default:
            break;
    }
}

void Fusion::handleTrackTitle(const J1939Message& msg,
        const Caster::Yield<Message>& yield) {
    if (handleString(msg) && track_title_.checksum(checksum_.value())) {
        yield(track_title_);
    }
}

void Fusion::handleTrackArtiat(const J1939Message& msg,
        const Caster::Yield<Message>& yield) {
    if (handleString(msg) && track_artist_.checksum(checksum_.value())) {
        yield(track_artist_);
    }
}

void Fusion::handleTrackAlbum(const J1939Message& msg,
        const Caster::Yield<Message>& yield) {
    if (handleString(msg) && track_album_.checksum(checksum_.value())) {
        yield(track_album_);
    }
}

void Fusion::handleTimeElapsed(const J1939Message& msg,
        const Caster::Yield<Message>& yield) {
    if (seq(msg) == 1) {
        uint32_t time = msg.data()[1];
        time |= (msg.data()[2] << 8);
        time |= (msg.data()[3] << 16);
        if (track_playback_.time_elapsed(time / 4)) {
            yield(track_playback_);
        }
    }
}

void Fusion::handleRadioFrequency(const J1939Message& msg,
        const Caster::Yield<Message>& yield) {
    if (seq(msg) == 1) {
        uint32_t frequency = msg.data()[1];
        frequency |= (msg.data()[2] << 8);
        frequency |= (msg.data()[3] << 16);
        frequency |= (msg.data()[4] << 24);
        if (system_.frequency(frequency))  {
            yield(system_);
        }
    }
}

void Fusion::handleInputGain(const J1939Message& msg,
        const Caster::Yield<Message>& yield) {
    if (seq(msg) == 0 && system_.gain((int8_t)msg.data()[7]))  {
        yield(system_);
    }
}

void Fusion::handleTone(const J1939Message& msg,
        const Caster::Yield<Message>& yield) {
    // We set all zones to the same EQ so we only care about reading the first
    // zone.
    if (msg.data()[6] != 0x00) {
        return;
    }
    bool changed = false;
    switch (seq(msg)) {
        case 0:
            changed |= tone_.bass(msg.data()[7]);
            break;
        case 1:
            changed |= tone_.mid(msg.data()[1]);
            changed |= tone_.treble(msg.data()[2]);
            break;
        default:
            break;
    }
    if (changed) {
        yield(tone_);
    }
}

void Fusion::handleMute(const J1939Message& msg,
        const Caster::Yield<Message>& yield) {
    if (seq(msg) == 0 && mute_.mute(msg.data()[6] != 0x00))  {
        yield(mute_);
    }
}

void Fusion::handleBalance(const J1939Message& msg,
        const Caster::Yield<Message>& yield) {
    // We balance all zones together so we only need to read
    // balance from zone 1.
    if (seq(msg) == 0 && msg.data()[6] == 0x00 && volume_.balance(msg.data()[7])) {
        yield(volume_);
    }
}

void Fusion::handleVolume(const J1939Message& msg,
        const Caster::Yield<Message>& yield) {
    // The second message contains the volume for zone 3. We don't use zone 3
    // because we're mimicing a car stereo with front/rear. So we only parse
    // the first message.
    if (seq(msg) == 0) {
        uint8_t zone1 = msg.data()[6];
        uint8_t zone2 = msg.data()[7];
        bool changed = false;
        if (zone1 > zone2) {
            changed |= volume_.volume(zone1);
        } else {
            changed |= volume_.volume(zone2);
        }
        changed |= volume_.fade(zone1 - zone2);
        if (recent_mute_) {
            changed = false;
            recent_mute_ = false;
        }
        if (changed) {
            yield(volume_);
        }
    }
}

void Fusion::handleBluetoothConnection(bool connected,
        const Caster::Yield<Message>& yield) {
    if (system_.bt_connected(connected)) {
        yield(system_);
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
    if (address_ == Canny::NullAddress) {
        // we can't send messages if we don't have an address
        return;
    }
    if (hu_address_ == Canny::NullAddress) {
        // periodically try to discover a stereo if we don't have one
        if (disco_timer_.active()) {
            Serial.println("disco timeout");
            sendStereoDiscovery(yield);
        }
    } else if (hb_timer_.active()) {
        // otherwise trigger loss of connectivity if no heartbeat has been received
        Serial.println("heartbeat timeout");
        hb_timer_.reset();
        hu_address_ = Canny::NullAddress;
        if (system_.available(false) || system_.power(false))  {
            yield(system_);
        }
    }
}

void Fusion::sendStereoRequest(const Caster::Yield<Message>& yield) {
    // 1DEF0A10#00:05:A3:99:1C:00:01:FF
    // 1DEF0A10#A0:04:A3:99:01:00:FF:FF
    J1939Message msg(0x1EF00, address_, hu_address_, 0x07);
    msg.data({0x00, 0x05, 0xA3, 0x99, 0x1C, 0x00, 0x01, 0xFF});
    yield(msg);
    msg.data({0xA0, 0x04, 0xA3, 0x99, 0x01, 0x00, 0xFF, 0xFF});
    yield(msg);
}

void Fusion::sendStereoDiscovery(const Caster::Yield<Message>& yield) {
    disco_timer_.reset();
    J1939Message msg(0xEAFF, address_, 0xFF, 0x06);
    msg.data({0x14, 0xF0, 0x01});
    yield(msg);
}

}  // namespace R51
