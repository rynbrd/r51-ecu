#include "Fusion.h"

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>
#include <Endian.h>
#include <Faker.h>
#include "Audio.h"

namespace R51 {
namespace {

template <typename T>
void clamp(T* var, T min, T max) {
    if (*var < min) {
        *var = min;
    } else if (*var > max) {
        *var = max;
    }
}

using ::Canny::J1939Message;
using ::Caster::Yield;
using ::Faker::Clock;

static const uint32_t kAvailabilityTimeout = 5000;
static const uint32_t kDiscoveryTick = 5000;
static const uint8_t kVolumeMax = 24;
static const int8_t kBalanceMin = -7;
static const int8_t kBalanceMax = 7;
static const int8_t kFadeMin = -8;
static const int8_t kFadeMax = 8;
static const int8_t kFadeMultiplier = 3;
static const int8_t kToneMin = -15;
static const int8_t kToneMax = 15;

enum State : uint8_t {
    // Unsupported state, reset counters.
    UNSUPPORTED = 0x00,

    // 4th byte of A3:99:XX:80 state frames.
    // 1DFF040A#C0:19:A3:99:11:80:07:00
    SOURCE = 0x02,
    TRACK_PLAYBACK = 0x04,
    TRACK_TITLE = 0x05,
    TRACK_ARTIST = 0x06,
    TRACK_ALBUM = 0x07,
    TRACK_ELAPSED = 0x09,
    RADIO_FREQUENCY = 0x0B,
    MENU_LOAD = 0x0F,
    MENU_ITEM_COUNT = 0x10,
    MENU_ITEM_LIST = 0x11,
    INPUT_GAIN = 0x13,
    MUTE = 0x17,
    BALANCE = 0x18,
    TONE = 0x1B,
    VOLUME = 0x1D,
    HEARTBEAT = 0x20,
    POWER = 0x39,

    // PGN 1F014 announcement
    ANNOUNCE = 0xF0,

    // Message is ignored.
    IGNORE = 0xFF,
};

enum TrackCmd : uint8_t {
    TRACK_CMD_PLAY = 0x01,
    TRACK_CMD_PAUSE = 0x02,
    TRACK_CMD_NEXT = 0x04,
    TRACK_CMD_PREV = 0x06,
};

enum RadioCmd : uint8_t {
    RADIO_CMD_NEXT_AUTO = 0x01,
    RADIO_CMD_NEXT_MANUAL = 0x02,
    RADIO_CMD_PREV_AUTO = 0x03,
    RADIO_CMD_PREV_MANUAL = 0x04,
    RADIO_CMD_TUNE = 0x05,
};

uint8_t counter(const J1939Message& msg) {
    return msg.data()[0];
}

uint8_t counter_id(uint8_t counter) {
    return (counter & 0xE0) >> 4;
}

uint8_t counter_id(const J1939Message& msg) {
    return counter_id(counter(msg));
}

uint8_t counter_seq(uint8_t counter) {
    return counter & 0x1F;
}

uint8_t counter_seq(const J1939Message& msg) {
    return counter_seq(counter(msg));
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
        return State::ANNOUNCE;
    } else if (msg.pgn() == 0x1FF04 && msg.source_address() == hu_address) {
        if (match(msg.data() + 2, {0xA3, 0x99, 0xFF, 0x80})) {
            return (State)msg.data()[4];
        }
        return State::UNSUPPORTED;
    }
    return State::IGNORE;
}

}  // namespace

Fusion::Fusion(Clock* clock) :
        clock_(clock), address_(Canny::NullAddress), hu_address_(Canny::NullAddress),
        hb_timer_(kAvailabilityTimeout, false, clock),
        disco_timer_(kDiscoveryTick, false, clock),
        recent_mute_(false), state_(0xFF), state_ignore_(false), state_counter_(0xFF),
        cmd_counter_(0x00), cmd_(0x1EF00, Canny::NullAddress),
        secondary_source_((AudioSource)0xFF) {
    cmd_.resize(8);
    track_title_.scratch = &track_title_scratch_;
    track_artist_.scratch = &track_artist_scratch_;
    track_album_.scratch = &track_album_scratch_;
    settings_item_.scratch = &settings_item_scratch_;
}

void Fusion::handle(const Message& msg, const Yield<Message>& yield) {
    switch (msg.type()) {
        case Message::EVENT:
            //TODO: Handle controller request command.
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

void Fusion::handleEvent(const Event& event, const Yield<Message>& yield) {
    if (address_ == Canny::NullAddress) {
        return;
    }
    switch ((AudioEvent)event.id) {
        case AudioEvent::POWER_ON_CMD:
            sendPowerCmd(yield, true);
            break;
        case AudioEvent::POWER_OFF_CMD:
            sendPowerCmd(yield, false);
            break;
        case AudioEvent::POWER_TOGGLE_CMD:
            sendPowerCmd(yield, !system_.power());
            break;
        case AudioEvent::SOURCE_SET_CMD:
            {
                auto* e = (AudioSourceSetCommand*)&event;
                sendSourceSetCmd(yield, e->source());
            }
            break;
        case AudioEvent::SOURCE_NEXT_CMD:
            handleSourceNextCmd(yield);
            break;
        case AudioEvent::SOURCE_PREV_CMD:
            handleSourcePrevCmd(yield);
            break;
        case AudioEvent::TRACK_PLAY_CMD:
            sendTrackCmd(yield, TRACK_CMD_PLAY);
            break;
        case AudioEvent::TRACK_PAUSE_CMD:
            sendTrackCmd(yield, TRACK_CMD_PAUSE);
            break;
        case AudioEvent::TRACK_NEXT_CMD:
            sendTrackCmd(yield, TRACK_CMD_NEXT);
            break;
        case AudioEvent::TRACK_PREV_CMD:
            sendTrackCmd(yield, TRACK_CMD_PREV);
            break;
        case AudioEvent::RADIO_TUNE_CMD:
            {
                auto* e = (AudioRadioTuneCommand*)&event;
                sendRadioCmd(yield, RADIO_CMD_TUNE, e->frequency());
            }
            break;
        case AudioEvent::RADIO_NEXT_AUTO_CMD:
            sendRadioCmd(yield, RADIO_CMD_NEXT_AUTO, system_.frequency());
            break;
        case AudioEvent::RADIO_PREV_AUTO_CMD:
            sendRadioCmd(yield, RADIO_CMD_PREV_AUTO, system_.frequency());
            break;
        case AudioEvent::RADIO_NEXT_MANUAL_CMD:
            sendRadioCmd(yield, RADIO_CMD_NEXT_MANUAL, system_.frequency());
            break;
        case AudioEvent::RADIO_PREV_MANUAL_CMD:
            sendRadioCmd(yield, RADIO_CMD_PREV_MANUAL, system_.frequency());
            break;
        case AudioEvent::RADIO_TOGGLE_SEEK_CMD:
            system_.toggle_seek_mode();
            yield(system_);
            break;
        case AudioEvent::RADIO_NEXT_CMD:
            switch (system_.seek_mode()) {
                case AudioSeek::AUTO:
                    sendRadioCmd(yield, RADIO_CMD_NEXT_AUTO, system_.frequency());
                    break;
                case AudioSeek::MANUAL:
                    sendRadioCmd(yield, RADIO_CMD_NEXT_MANUAL, system_.frequency());
                    break;
            }
            break;
        case AudioEvent::RADIO_PREV_CMD:
            switch (system_.seek_mode()) {
                case AudioSeek::AUTO:
                    sendRadioCmd(yield, RADIO_CMD_PREV_AUTO, system_.frequency());
                    break;
                case AudioSeek::MANUAL:
                    sendRadioCmd(yield, RADIO_CMD_NEXT_MANUAL, system_.frequency());
                    break;
            }
            break;
        case AudioEvent::INPUT_GAIN_SET_CMD:
            {
                auto* e = (AudioInputGainSetCommand*)&event;
                sendInputGainSetCmd(yield, e->gain());
            }
            break;
        case AudioEvent::INPUT_GAIN_INC_CMD:
            sendInputGainSetCmd(yield, system_.gain() + 1);
            break;
        case AudioEvent::INPUT_GAIN_DEC_CMD:
            sendInputGainSetCmd(yield, system_.gain() - 1);
            break;
        case AudioEvent::VOLUME_SET_CMD:
            {
                auto* e = (AudioVolumeSetCommand*)&event;
                sendVolumeSetCmd(yield, e->volume(), volume_.fade());
            }
            break;
        case AudioEvent::VOLUME_INC_CMD:
            sendVolumeSetCmd(yield, volume_.volume() + 1, volume_.fade());
            break;
        case AudioEvent::VOLUME_DEC_CMD:
            sendVolumeSetCmd(yield, volume_.volume() - 1, volume_.fade());
            break;
        case AudioEvent::VOLUME_MUTE_CMD:
            sendVolumeMuteCmd(yield, true);
            break;
        case AudioEvent::VOLUME_UNMUTE_CMD:
            sendVolumeMuteCmd(yield, false);
            break;
        case AudioEvent::VOLUME_TOGGLE_MUTE_CMD:
            sendVolumeMuteCmd(yield, !volume_.mute());
            break;
        case AudioEvent::BALANCE_SET_CMD:
            {
                auto* e = (AudioBalanceSetCommand*)&event;
                sendBalanceSetCmd(yield, e->balance());
            }
            break;
        case AudioEvent::BALANCE_LEFT_CMD:
            sendBalanceSetCmd(yield, volume_.balance() - 1);
            break;
        case AudioEvent::BALANCE_RIGHT_CMD:
            sendBalanceSetCmd(yield, volume_.balance() + 1);
            break;
        case AudioEvent::FADE_SET_CMD:
            {
                auto* e = (AudioFadeSetCommand*)&event;
                sendVolumeSetCmd(yield, volume_.volume(), e->fade());
            }
            break;
        case AudioEvent::FADE_FRONT_CMD:
            sendVolumeSetCmd(yield, volume_.volume(), volume_.fade() + 1);
            break;
        case AudioEvent::FADE_REAR_CMD:
            sendVolumeSetCmd(yield, volume_.volume(), volume_.fade() - 1);
            break;
        case AudioEvent::TONE_SET_CMD:
            {
                auto* e = (AudioToneSetCommand*)&event;
                sendToneSetCmd(yield, e->bass(), e->mid(), e->treble());
            }
            break;
        case AudioEvent::TONE_BASS_INC_CMD:
            sendToneSetCmd(yield, tone_.bass() + 1, tone_.mid(), tone_.treble());
            break;
        case AudioEvent::TONE_BASS_DEC_CMD:
            sendToneSetCmd(yield, tone_.bass() - 1, tone_.mid(), tone_.treble());
            break;
        case AudioEvent::TONE_MID_INC_CMD:
            sendToneSetCmd(yield, tone_.bass(), tone_.mid() + 1, tone_.treble());
            break;
        case AudioEvent::TONE_MID_DEC_CMD:
            sendToneSetCmd(yield, tone_.bass(), tone_.mid() - 1, tone_.treble());
            break;
        case AudioEvent::TONE_TREBLE_INC_CMD:
            sendToneSetCmd(yield, tone_.bass(), tone_.mid(), tone_.treble() + 1);
            break;
        case AudioEvent::TONE_TREBLE_DEC_CMD:
            sendToneSetCmd(yield, tone_.bass(), tone_.mid(), tone_.treble() - 1);
            break;
        case AudioEvent::PLAYBACK_TOGGLE_CMD:
            handlePlaybackToggleCmd(yield);
            break;
        case AudioEvent::PLAYBACK_NEXT_CMD:
            handlePlaybackNextCmd(yield);
            break;
        case AudioEvent::PLAYBACK_PREV_CMD:
            handlePlaybackPrevCmd(yield);
            break;
        case AudioEvent::SETTINGS_OPEN_CMD:
            sendMenuSettings(yield);
            break;
        case AudioEvent::SETTINGS_SELECT_CMD:
            {
                auto* e = (AudioSettingsSelectCommand*)&event;
                sendMenuSelectItem(yield, e->item());
            }
            break;
        case AudioEvent::SETTINGS_BACK_CMD:
            if (settings_menu_.page() == 0x01)  {
                yield(settings_exit_);
                sendMenuExit(yield);
            } else {
                sendMenuBack(yield);
            }
            break;
        case AudioEvent::SETTINGS_EXIT_CMD:
            sendMenuExit(yield);
            break;
        default:
            break;
    }
}

void Fusion::handleJ1939Claim(const J1939Claim& claim, const Yield<Message>& yield) {
    address_ = claim.address();
    cmd_.source_address(address_);
    if (address_ != Canny::NullAddress && hu_address_ == Canny::NullAddress) {
        sendStereoDiscovery(yield);
    }
}

void Fusion::handleJ1939Message(const J1939Message& msg, const Yield<Message>& yield) {
    if (address_ == Canny::NullAddress) {
        return;
    }

    if (counter_id(msg) != counter_id(state_counter_)) {
        State next_state = detectState(msg, hu_address_);
        if (next_state == IGNORE) {
           return;
        } 
        state_ = next_state;
        state_ignore_ = false;
    }
    state_counter_ = counter(msg);
    if (state_ignore_) {
        return;
    }
    handleState(counter_seq(msg), msg, yield);
}

void Fusion::handleState(uint8_t seq, const J1939Message& msg, const Yield<Message>& yield) {
    switch (state_) {
        case SOURCE:
            handleSource(seq, msg, yield);
            break;
        case TRACK_PLAYBACK:
            handleTrackPlayback(seq, msg, yield);
            break;
        case TRACK_TITLE:
            handleTrackString(&track_title_scratch_, seq, msg, &track_title_, yield);
            break;
        case TRACK_ARTIST:
            handleTrackString(&track_artist_scratch_, seq, msg, &track_artist_, yield);
            break;
        case TRACK_ALBUM:
            handleTrackString(&track_album_scratch_, seq, msg, &track_album_, yield);
            break;
        case TRACK_ELAPSED:
            handleTimeElapsed(seq, msg, yield);
            break;
        case RADIO_FREQUENCY:
            handleRadioFrequency(seq, msg, yield);
            break;
        case INPUT_GAIN:
            handleInputGain(seq, msg, yield);
            break;
        case TONE:
            handleTone(seq, msg, yield);
            break;
        case MUTE:
            handleMute(seq, msg, yield);
            break;
        case BALANCE:
            handleBalance(seq, msg, yield);
            break;
        case VOLUME:
            handleVolume(seq, msg, yield);
            break;
        case HEARTBEAT:
            handleHeartbeat(seq, msg, yield);
            break;
        case POWER:
            handlePower(seq, msg, yield);
            break;
        case MENU_LOAD:
            handleMenuLoad(seq, msg, yield);
            break;
        case MENU_ITEM_COUNT:
            handleMenuItemCount(seq, msg, yield);
            break;
        case MENU_ITEM_LIST:
            handleMenuItemList(seq, msg, yield);
            break;
        case ANNOUNCE:
            handleAnnounce(seq, msg, yield);
            break;
    }
}

void Fusion::handleAnnounce(uint8_t seq, const Canny::J1939Message& msg,
        const Yield<Message>& yield) {
    // 19F0140A#A0:86:35:08:8E:12:4D:53
    if (seq != 0 || !match(msg.data() + 1, {0x86, 0x35})) {
        return;
    }
    hu_address_ = msg.source_address();
    cmd_.dest_address(hu_address_);
    hb_timer_.reset();
    disco_timer_.reset();
    if (system_.available(true) || system_.power(false)) {
        yield(system_);
    }
    sendStereoRequest(yield);
}

void Fusion::handlePower(uint8_t seq, const Canny::J1939Message& msg,
        const Yield<Message>& yield) {
    if (seq != 0) {
        return;
    }
    if (system_.power(msg.data()[6] != 0x00)) {
        yield(system_);
    }
}

void Fusion::handleHeartbeat(uint8_t seq, const Canny::J1939Message& msg,
        const Yield<Message>& yield) {
    if (seq != 0) {
        return;
    }
    hb_timer_.reset();
    if (system_.power(msg.data()[6] == 0x01)) {
        yield(system_);
    }
}

void Fusion::handleSource(uint8_t seq, const J1939Message& msg,
        const Yield<Message>& yield) {
    switch (seq) {
        case 0:
            secondary_source_ = (AudioSource)msg.data()[6];
            if (msg.data()[6] == msg.data()[7] && system_.source((AudioSource)msg.data()[7])) {
                yield(system_);
            }
            break;
        case 1:
            if (secondary_source_ == AudioSource::BLUETOOTH) {
                switch (msg.data()[2]) {
                    case 0xA5:
                        handleBluetoothConnection(true, yield);
                        break;
                    case 0x85:
                        handleBluetoothConnection(false, yield);
                        break;
                    default:
                        break;
                }
            }
            break;
        default:
            break;
    }
}

void Fusion::handleTrackPlayback(uint8_t seq, const J1939Message& msg,
        const Yield<Message>& yield) {
    uint32_t time;
    switch (seq) {
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

void Fusion::handleTrackString(Scratch* scratch, uint8_t seq, const J1939Message& msg,
        AudioChecksumEvent* event, const Yield<Message>& yield) {
    if (handleString(scratch, seq, msg, 4) && event->checksum(checksum_.value())) {
        yield(*event);
    }
}

void Fusion::handleTimeElapsed(uint8_t seq, const J1939Message& msg,
        const Yield<Message>& yield) {
    if (seq == 1) {
        uint32_t time = msg.data()[1];
        time |= (msg.data()[2] << 8);
        time |= (msg.data()[3] << 16);
        if (track_playback_.time_elapsed(time / 4)) {
            yield(track_playback_);
        }
    }
}

void Fusion::handleRadioFrequency(uint8_t seq, const J1939Message& msg,
        const Yield<Message>& yield) {
    if (seq == 1) {
        uint32_t frequency = msg.data()[1];
        frequency |= (msg.data()[2] << 8);
        frequency |= (msg.data()[3] << 16);
        frequency |= (msg.data()[4] << 24);
        if (system_.frequency(frequency))  {
            yield(system_);
        }
    }
}

void Fusion::handleInputGain(uint8_t seq, const J1939Message& msg,
        const Yield<Message>& yield) {
    if (seq == 0 && system_.gain((int8_t)msg.data()[7]))  {
        yield(system_);
    }
}

void Fusion::handleTone(uint8_t seq, const J1939Message& msg,
        const Yield<Message>& yield) {
    // We set all zones to the same EQ so we only care about reading the first
    // zone.
    bool changed = false;
    switch (seq) {
        case 0:
            if (msg.data()[6] != 0x00) {
                state_ignore_ = true;
                return;
            }
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

void Fusion::handleMute(uint8_t seq, const J1939Message& msg,
        const Yield<Message>& yield) {
    if (seq == 0 && volume_.mute(msg.data()[6] == 0x01))  {
        yield(volume_);
    }
}

void Fusion::handleBalance(uint8_t seq, const J1939Message& msg,
        const Yield<Message>& yield) {
    // We balance all zones together so we only need to read
    // balance from zone 1.
    if (seq == 0 && msg.data()[6] == 0x00 && volume_.balance(msg.data()[7])) {
        yield(volume_);
    }
}

void Fusion::handleVolume(uint8_t seq, const J1939Message& msg,
        const Yield<Message>& yield) {
    // The second message contains the volume for zone 3. We don't use zone 3
    // because we're mimicing a car stereo with front/rear. So we only parse
    // the first message.
    if (seq == 0) {
        uint8_t zone1 = msg.data()[6];
        uint8_t zone2 = msg.data()[7];
        bool changed = false;
        if (zone1 > zone2) {
            changed |= volume_.volume(zone1);
        } else {
            changed |= volume_.volume(zone2);
        }
        changed |= volume_.fade((zone1 - zone2) / kFadeMultiplier);
        if (recent_mute_) {
            changed = false;
            recent_mute_ = false;
        }
        if (changed) {
            yield(volume_);
        }
    }
}

void Fusion::handleMenuLoad(uint8_t seq, const Canny::J1939Message& msg,
        const Yield<Message>& yield) {
    switch (seq) {
        case 0:
            settings_menu_.item(msg.data()[7]);
            break;
        case 1:
            settings_menu_.page(msg.data()[4]);
            switch (msg.data()[4]) {
                case 0x01:
                case 0x02:
                    // load menu page
                    sendMenuReqItemCount(yield);
                    break;
                case 0x03:
                    // refresh menu item
                    break;
                case 0x04:
                    {
                        Event event(SubSystem::AUDIO, (uint8_t)AudioEvent::SETTINGS_EXIT_STATE);
                        yield(event);
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void Fusion::handleMenuItemCount(uint8_t seq, const Canny::J1939Message& msg,
        const Yield<Message>& yield) {
    // 1DFF040A#20:0A:A3:99:10:80:07:03
    //                                |
    //                                +- count
    // 1DFF040A#21:00:00:00:03:FF:FF:FF
    if (seq == 0) {
        uint8_t count = msg.data()[7];
        if (count > 5) {
            count = 5;
        }
        settings_menu_.count(count);
        yield(settings_menu_);
        sendMenuReqItemList(yield, count);
    }
}

void Fusion::handleMenuItemList(uint8_t seq, const Canny::J1939Message& msg,
        const Yield<Message>& yield) {
    // 1DFF040A#60:19:A3:99:11:80:07:00
    // 1DFF040A#61:00:00:00:89:03:0C:44
    // 1DFF040A#62:69:73:63:6F:76:65:72
    // 1DFF040A#63:61:62:6C:65:00:FF:FF
    switch (seq) {
        case 0:
            settings_item_.reload(settings_menu_.page() == 0x03);
            settings_item_.item(msg.data()[7]);
            break;
        case 1:
            switch (msg.data()[4]) {
                case 0x49:
                    settings_item_.type(AudioSettingsType::SUBMENU);
                    break;
                case 0x011:
                    settings_item_.type(AudioSettingsType::SELECT);
                    break;
                case 0x81:
                case 0x89:
                    settings_item_.type(AudioSettingsType::CHECKBOX_OFF);
                    break;
                case 0x83:
                case 0x8B:
                    settings_item_.type(AudioSettingsType::CHECKBOX_ON);
                    break;
            }
            break;
    }
    if (handleString(&settings_item_scratch_, seq, msg, 6)) {
        yield(settings_item_);
    }
}

void Fusion::handleBluetoothConnection(bool connected,
        const Yield<Message>& yield) {
    if (system_.bt_connected(connected)) {
        yield(system_);
    }
}

void Fusion::handleSourceNextCmd(const Caster::Yield<Message>& yield) {
    if (!system_.power()) {
        return;
    }
    switch (system_.source()) {
        case AudioSource::BLUETOOTH:
            sendSourceSetCmd(yield, AudioSource::AM);
            break;
        case AudioSource::AM:
            sendSourceSetCmd(yield, AudioSource::FM);
            break;
        default:
        case AudioSource::FM:
            sendSourceSetCmd(yield, AudioSource::BLUETOOTH);
            break;
    }
}

void Fusion::handleSourcePrevCmd(const Caster::Yield<Message>& yield) {
    if (!system_.power()) {
        return;
    }
    switch (system_.source()) {
        default:
        case AudioSource::BLUETOOTH:
            sendSourceSetCmd(yield, AudioSource::FM);
            break;
        case AudioSource::AM:
            sendSourceSetCmd(yield, AudioSource::BLUETOOTH);
            break;
        case AudioSource::FM:
            sendSourceSetCmd(yield, AudioSource::AM);
            break;
    }
}

void Fusion::handlePlaybackToggleCmd(const Caster::Yield<Message>& yield) {
    if (!system_.power()) {
        return;
    }
    switch (system_.source()) {
        case AudioSource::AM:
        case AudioSource::FM:
            system_.toggle_seek_mode();
            yield(system_);
            break;
        case AudioSource::BLUETOOTH:
            if (track_playback_.playback() == AudioPlayback::PAUSE) {
                sendTrackCmd(yield, TRACK_CMD_PLAY);
            } else {
                sendTrackCmd(yield, TRACK_CMD_PAUSE);
            }
            break;
        default:
            break;
    }
}

void Fusion::handlePlaybackNextCmd(const Caster::Yield<Message>& yield) {
    if (!system_.power()) {
        return;
    }
    switch (system_.source()) {
        case AudioSource::AM:
        case AudioSource::FM:
            switch (system_.seek_mode()) {
                case AudioSeek::AUTO:
                    sendRadioCmd(yield, RADIO_CMD_NEXT_AUTO, system_.frequency());
                    break;
                case AudioSeek::MANUAL:
                    sendRadioCmd(yield, RADIO_CMD_NEXT_MANUAL, system_.frequency());
                    break;
            }
            break;
        case AudioSource::BLUETOOTH:
            sendTrackCmd(yield, TRACK_CMD_NEXT);
            break;
        case AudioSource::AUX:
        case AudioSource::OPTICAL:
            sendInputGainSetCmd(yield, system_.gain() + 1);
            break;
        default:
            break;
    }
}

void Fusion::handlePlaybackPrevCmd(const Caster::Yield<Message>& yield) {
    if (!system_.power()) {
        return;
    }
    switch (system_.source()) {
        case AudioSource::AM:
        case AudioSource::FM:
            switch (system_.seek_mode()) {
                case AudioSeek::AUTO:
                    sendRadioCmd(yield, RADIO_CMD_PREV_AUTO, system_.frequency());
                    break;
                case AudioSeek::MANUAL:
                    sendRadioCmd(yield, RADIO_CMD_PREV_MANUAL, system_.frequency());
                    break;
            }
            break;
        case AudioSource::BLUETOOTH:
            sendTrackCmd(yield, TRACK_CMD_PREV);
            break;
        case AudioSource::AUX:
        case AudioSource::OPTICAL:
            sendInputGainSetCmd(yield, system_.gain() - 1);
            break;
        default:
            break;
    }
}

bool Fusion::handleString(Scratch* scratch, uint8_t seq, const J1939Message& msg, uint8_t offset) {
    if (seq == 0) {
        scratch->bytes[0] = 0;
        scratch->size = 0;
        checksum_.reset();
        return false;
    }

    uint8_t i = 0;
    if (seq == 1) {
        // skip prefix in first frame
        i = offset;
    }
    for (; i < 7; i++) {
        if (scratch->size >= 256) {
            // buffer overflow
            return false;
        }

        scratch->bytes[scratch->size] = msg.data()[i + 1];
        checksum_.update(msg.data()[i + 1]);
        if (scratch->bytes[scratch->size] == 0) {
            // end of string
            return true;
        }
        ++(scratch->size);
    }
    return false;
}

void Fusion::emit(const Yield<Message>& yield) {
    if (address_ == Canny::NullAddress) {
        // we can't send messages if we don't have an address
        return;
    }
    if (hu_address_ == Canny::NullAddress) {
        // periodically try to discover a stereo if we don't have one
        if (disco_timer_.active()) {
            sendStereoDiscovery(yield);
        }
    } else if (hb_timer_.active()) {
        // otherwise trigger loss of connectivity if no heartbeat has been received
        hb_timer_.reset();
        hu_address_ = Canny::NullAddress;
        cmd_.dest_address(Canny::NullAddress);
        if (system_.available(false) || system_.power(false))  {
            yield(system_);
        }
    }
}

void Fusion::sendStereoRequest(const Yield<Message>& yield) {
    // 1DEF0A10#00:05:A3:99:1C:00:01:FF
    // 1DEF0A10#A0:04:A3:99:01:00:FF:FF
    sendCmd(yield, 0x05, 0x1C, 0x01);
    sendCmd(yield, 0x04, 0x01);
}

void Fusion::sendStereoDiscovery(const Yield<Message>& yield) {
    disco_timer_.reset();
    J1939Message msg(0xEAFF, address_, 0xFF, 0x06);
    msg.data({0x14, 0xF0, 0x01});
    yield(msg);
}

void Fusion::sendCmd(const Yield<Message>& yield,
        uint8_t cs, uint8_t id, uint8_t payload0, uint8_t payload1) {
    cmd_counter_ = (cmd_counter_ & 0xE0) + 0x20;
    cmd_.data({cmd_counter_++, cs, 0xA3, 0x99, id, 0x00, payload0, payload1});
    yield(cmd_);
}

void Fusion::sendCmdPayload(const Yield<Message>& yield, uint32_t payload) {
    cmd_.data()[0] = cmd_counter_++;
    // Fusion expects ints in little-endian byte order.
    Endian::hltob(cmd_.data() + 1, payload, Endian::LITTLE);
    memset(cmd_.data() + 5, 0xFF, 3);
    yield(cmd_);
}


void Fusion::sendPowerCmd(const Yield<Message>& yield, bool power) {
    // on:  1DEF0A21#C0:05:A3:99:1C:00:01:FF
    // off: 1DEF0A21#40:05:A3:99:1C:00:02:FF
    uint8_t power_byte = power ? 0x01 : 0x02;
    sendCmd(yield, 0x05, 0x1C, power_byte);
}

void Fusion::sendSourceSetCmd(const Yield<Message>& yield, AudioSource source) {
    // 1DEF0A21#E0:05:A3:99:02:00:00:FF
    //                             |
    //                             +---- source
    sendCmd(yield, 0x05, 0x02, (uint8_t)source);
}

void Fusion::sendTrackCmd(const Yield<Message>& yield, uint8_t cmd) {
    // play:  1DEF0A21#A0:06:A3:99:03:00:07:01
    // pause: 1DEF0A21#C0:06:A3:99:03:00:07:02
    // next:  1DEF0A21#00:06:A3:99:03:00:07:04
    // prev:  1DEF0A21#20:06:A3:99:03:00:07:06
    sendCmd(yield, 0x06, 0x03, 0x07, cmd);
}

void Fusion::sendRadioCmd(const Yield<Message>& yield, uint8_t cmd, uint32_t freq) {
    // 1DEF0A21#40:0A:A3:99:05:00:00:01 next auto
    // 1DEF0A21#E0:0A:A3:99:05:00:00:02 next manual
    // 1DEF0A21#C0:0A:A3:99:05:00:00:03 prev auto
    // 1DEF0A21#20:0A:A3:99:05:00:00:04 prev manual
    // 1DEF0A21#20:0A:A3:99:05:00:00:05 tune to frequency
    //                             |
    //                             +---- source: am/fm
    // 1DEF0A21#41:50:16:08:00:FF:FF:FF current or desired freq
    sendCmd(yield, 0x0A, 0x05, (uint8_t)system_.source(), cmd);
    sendCmdPayload(yield, freq);
}

void Fusion::sendInputGainSetCmd(const Yield<Message>& yield, int8_t gain) {
    // 1DEF0A21#60:06:A3:99:0D:00:03:01
    //                             |  |
    //                             |  +- gain (signed)
    //                             +---- source
    sendCmd(yield, 0x06, 0x0D, (uint8_t)system_.source(), gain);
}

void Fusion::sendVolumeSetCmd(const Yield<Message>& yield, uint8_t volume, int8_t fade) {
    // 1DEF0A21#80:08:A3:99:19:00:11:0E
    //                             |  |
    //                             |  +- zone 2
    //                             +---- zone 1
    // 1DEF0A21#81:0A:01:FF:FF:FF:FF:FF
    //              |
    //              +------------------- zone 3

    if (fade < kFadeMin) {
        fade = kFadeMin;
    } else if (fade > kFadeMax) {
        fade = kFadeMax;
    }
    fade *= kFadeMultiplier;
    int8_t zone1 = (int8_t)volume;
    int8_t zone2 = (int8_t)volume;
    if (fade < 0) {
        // fade to back; reduce front volume
        zone1 += fade;
        clamp<int8_t>(&zone1, 0, kVolumeMax);
    } else {
        // fade to front; reduce rear volume
        zone2 -= fade;
        clamp<int8_t>(&zone2, 0, kVolumeMax);
    }

    sendCmd(yield, 0x08, 0x19, zone1, zone2);
    sendCmdPayload(yield, {volume, 0x01});
}

void Fusion::sendVolumeMuteCmd(const Yield<Message>& yield, bool mute) {
    // mute:   1DEF0A21#40:05:A3:99:11:00:01:FF
    // unmute: 1DEF0A21#60:05:A3:99:11:00:02:FF
    sendCmd(yield, 0x05, 0x11, mute ? 0x01 : 0x02);
}

void Fusion::sendBalanceSetCmd(const Yield<Message>& yield, int8_t balance) {
    // 1DEF0A21#00:06:A4:99:12:00:00:01
    //                                |
    //                                +- balance (signed)
    if (balance < kBalanceMin) {
        balance = kBalanceMin;
    } else if (balance > kBalanceMax) {
        balance = kBalanceMax;
    }
    // Sync balance in front and rear zones.
    sendCmd(yield, 0x06, 0x12, 0x00, balance);
    sendCmd(yield, 0x06, 0x12, 0x01, balance);
}

void Fusion::sendToneSetCmd(const Caster::Yield<Message>& yield,
        int8_t bass, int8_t mid, int8_t treble) {
    // 7(21->0A)EF00#40:08:A3:99:16:00:00:F1
    //                                  |  +-- bass
    //                                  +----- zone
    // 7(21->0A)EF00#41:01:0E:FF:FF:FF:FF:FF
    //                   |  +----------------- treble
    //                   +-------------------- mid
    clamp(&bass, kToneMin, kToneMax);
    clamp(&mid, kToneMin, kToneMax);
    clamp(&treble, kToneMin, kToneMax);
    sendCmd(yield, 0x08, 0x16, 0x00, bass);
    sendCmdPayload(yield, {(uint8_t)mid, (uint8_t)treble});
    sendCmd(yield, 0x08, 0x16, 0x01, bass);
    sendCmdPayload(yield, {(uint8_t)mid, (uint8_t)treble});
}

void Fusion::sendMenu(const Yield<Message>& yield, uint8_t page, uint8_t item) {
    sendCmd(yield, 0x0B, 0x09, (uint8_t)system_.source(), item);
    sendCmdPayload(yield, {0x00, 0x00, 0x00, page, 0x03});
}

void Fusion::sendMenuSettings(const Yield<Message>& yield) {
    // 1DEF0A21#40:0B:A3:99:09:00:07:00
    // 1DEF0A21#41:00:00:00:01:03:FF:FF
    sendMenu(yield, 0x01, 0x00);
}

void Fusion::sendMenuSelectItem(const Yield<Message>& yield, uint8_t item) {
    // 1DEF0A21#80:0B:A3:99:09:00:07:01
    // 1DEF0A21#81:00:00:00:02:03:FF:FF
    sendMenu(yield, 0x02, item);
}

void Fusion::sendMenuBack(const Yield<Message>& yield) {
    // 1DEF0A21#A0:0B:A3:99:09:00:07:00
    // 1DEF0A21#A1:00:00:00:03:03:FF:FF
    sendMenu(yield, 0x03, 0x00);
}
    
void Fusion::sendMenuExit(const Yield<Message>& yield) {
    // 1DEF0A21#40:0B:A3:99:09:00:07:00
    // 1DEF0A21#41:00:00:00:04:03:FF:FF
    sendMenu(yield, 0x04, 0x00);
}

void Fusion::sendMenuReqItemCount(const Yield<Message>& yield) {
    // 1DEF0A21#40:06:A3:99:0A:00:07:03
    sendCmd(yield, 0x06, 0x0A, (uint8_t)system_.source(), 0x03);
}

void Fusion::sendMenuReqItemList(const Yield<Message>& yield, uint8_t count) {
    // 1DEF0A21#60:0E:A3:99:0B:00:07:00
    // 1DEF0A21#61:00:00:00:03:00:00:00
    //                       |
    //                       +---------- item count
    // 1DEF0A21#82:03:FF:FF:FF:FF:FF:FF
    sendCmd(yield, 0x0E, 0x0B, (uint8_t)system_.source(), 0x00);
    sendCmdPayload(yield, {0x00, 0x00, 0x00, count, 0x00, 0x00, 0x00});
    sendCmdPayload(yield, {0x03});
}

}  // namespace R51
