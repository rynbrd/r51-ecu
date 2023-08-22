#include "Fusion.h"

#include <Arduino.h>
#include <ByteOrder.h>
#include <Caster.h>
#include <Core.h>
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

static const uint32_t kBootInitTimeout = 500;
static const uint32_t kBootRequestTimeout = 5000;
static const uint32_t kHeartbeatTimeout = 5000;
static const uint32_t kDiscoveryTick = 5000;
static const int8_t kFadeMultiplier = 3;

enum BootState : uint8_t {
    UNKNOWN = 0,    // not in a boot mode
    DISCOVERED = 1, // stereo discovered, processing discovery response
    ANNOUNCED = 2,  // announced ourselves, processing announcement response
    REQUESTED = 3,  // requested initial state from stereo, waiting for all of it
    POWERING = 4,   // powering on and waiting for source
};

// Mapping of Fusion state identifiers.
enum FusionState : uint8_t {
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

    // Response to J1939 request messages.
    PGN_01F014 = 0xF4,
    PGN_01F016 = 0xF6,

    // Invalid state detected.
    INVALID = 0xFF,
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

}  // namespace

Fusion::Fusion(Clock* clock) :
        clock_(clock), address_(Canny::NullAddress), hu_address_(Canny::NullAddress),
        boot_state_(UNKNOWN),
        disco_timer_(kDiscoveryTick, false, clock),
        boot_timer_(kBootInitTimeout, true, clock),
        state_(0xFF), state_ignore_next_(false), state_pgn_(0), state_counter_(0xFF),
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
            if ((RequestCommand::match(*msg.event(), SubSystem::AUDIO,
                    (uint8_t)AudioEvent::SYSTEM_STATE))) {
                yield(MessageView(&system_));
            }
            if ((RequestCommand::match(*msg.event(), SubSystem::AUDIO,
                    (uint8_t)AudioEvent::VOLUME_STATE))) {
                yield(MessageView(&volume_));
            }
            if ((RequestCommand::match(*msg.event(), SubSystem::AUDIO,
                    (uint8_t)AudioEvent::TONE_STATE))) {
                yield(MessageView(&tone_));
            }
            if ((RequestCommand::match(*msg.event(), SubSystem::AUDIO,
                    (uint8_t)AudioEvent::SOURCE_STATE))) {
                yield(MessageView(&source_));
            }
            if ((RequestCommand::match(*msg.event(), SubSystem::AUDIO,
                    (uint8_t)AudioEvent::TRACK_PLAYBACK_STATE))) {
                yield(MessageView(&track_playback_));
            }
            if ((RequestCommand::match(*msg.event(), SubSystem::AUDIO,
                    (uint8_t)AudioEvent::TRACK_TITLE_STATE))) {
                yield(MessageView(&track_title_));
            }
            if ((RequestCommand::match(*msg.event(), SubSystem::AUDIO,
                    (uint8_t)AudioEvent::TRACK_ARTIST_STATE))) {
                yield(MessageView(&track_artist_));
            }
            if ((RequestCommand::match(*msg.event(), SubSystem::AUDIO,
                    (uint8_t)AudioEvent::TRACK_ALBUM_STATE))) {
                yield(MessageView(&track_album_));
            }
            if ((RequestCommand::match(*msg.event(), SubSystem::AUDIO,
                    (uint8_t)AudioEvent::RADIO_STATE))) {
                yield(MessageView(&radio_));
            }
            if ((RequestCommand::match(*msg.event(), SubSystem::AUDIO,
                    (uint8_t)AudioEvent::INPUT_STATE))) {
                yield(MessageView(&input_));
            }
            if (msg.event()->subsystem == (uint8_t)SubSystem::AUDIO) {
                handleCommand(*msg.event(), yield);
            }
            break;
        case Message::J1939_CLAIM:
            handleJ1939Claim(*msg.j1939_claim(), yield);
            break;
        case Message::J1939_MESSAGE:
            handleJ1939Message(*msg.j1939_message(), yield);
            break;
        default:
            break;
    }
}

void Fusion::handleCommand(const Event& event, const Yield<Message>& yield) {
    if (system_.state() == AudioSystem::OFF) {
        if (event.id == (uint8_t)AudioEvent::POWER_ON_CMD ||
                event.id == (uint8_t)AudioEvent::POWER_TOGGLE_CMD) {
            sendPowerCmd(yield, true);
        }
        return;
    } else if (system_.state() == AudioSystem::BOOT ||
            system_.state() == AudioSystem::POWER_ON) {
        if (event.id == (uint8_t)AudioEvent::POWER_OFF_CMD ||
                event.id == (uint8_t)AudioEvent::POWER_TOGGLE_CMD) {
            sendPowerCmd(yield, false);
        }
        return;
    } else if (system_.state() != AudioSystem::ON) {
        return;
    }

    switch ((AudioEvent)event.id) {
        // Power commands.
        case AudioEvent::POWER_OFF_CMD:
        case AudioEvent::POWER_TOGGLE_CMD:
            sendPowerCmd(yield, false);
            break;

        // Source commands.
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

        // Bluetooth source commands.
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

        // Radio source commands.
        case AudioEvent::RADIO_TUNE_CMD:
            {
                auto* e = (AudioRadioTuneCommand*)&event;
                sendRadioCmd(yield, RADIO_CMD_TUNE, e->frequency());
            }
            break;
        case AudioEvent::RADIO_NEXT_AUTO_CMD:
            sendRadioCmd(yield, RADIO_CMD_NEXT_AUTO, radio_.frequency());
            break;
        case AudioEvent::RADIO_PREV_AUTO_CMD:
            sendRadioCmd(yield, RADIO_CMD_PREV_AUTO, radio_.frequency());
            break;
        case AudioEvent::RADIO_NEXT_MANUAL_CMD:
            sendRadioCmd(yield, RADIO_CMD_NEXT_MANUAL, radio_.frequency());
            break;
        case AudioEvent::RADIO_PREV_MANUAL_CMD:
            sendRadioCmd(yield, RADIO_CMD_PREV_MANUAL, radio_.frequency());
            break;
        case AudioEvent::RADIO_TOGGLE_SEEK_CMD:
            radio_.toggle_seek_mode();
            yield(MessageView(&radio_));
            break;
        case AudioEvent::RADIO_NEXT_CMD:
            switch (radio_.seek_mode()) {
                case AudioSeek::AUTO:
                    sendRadioCmd(yield, RADIO_CMD_NEXT_AUTO, radio_.frequency());
                    break;
                case AudioSeek::MANUAL:
                    sendRadioCmd(yield, RADIO_CMD_NEXT_MANUAL, radio_.frequency());
                    break;
            }
            break;
        case AudioEvent::RADIO_PREV_CMD:
            switch (radio_.seek_mode()) {
                case AudioSeek::AUTO:
                    sendRadioCmd(yield, RADIO_CMD_PREV_AUTO, radio_.frequency());
                    break;
                case AudioSeek::MANUAL:
                    sendRadioCmd(yield, RADIO_CMD_NEXT_MANUAL, radio_.frequency());
                    break;
            }
            break;

        // Aux and Optical input source commands.
        case AudioEvent::INPUT_GAIN_SET_CMD:
            {
                auto* e = (AudioInputGainSetCommand*)&event;
                sendInputGainSetCmd(yield, e->gain());
            }
            break;
        case AudioEvent::INPUT_GAIN_INC_CMD:
            sendInputGainSetCmd(yield, input_.gain() + 1);
            break;
        case AudioEvent::INPUT_GAIN_DEC_CMD:
            sendInputGainSetCmd(yield, input_.gain() - 1);
            break;

        // Volume commands.
        case AudioEvent::VOLUME_SET_CMD:
            {
                auto* e = (AudioVolumeSetCommand*)&event;
                sendVolumeSetCmd(yield, e->volume(), volume_.fade());
            }
            break;
        case AudioEvent::VOLUME_INC_CMD:
            if (volume_.volume() < kVolumeMax) {
                sendVolumeSetCmd(yield, volume_.volume() + 1, volume_.fade());
            } else {
                sendVolumeSetCmd(yield, volume_.volume(), volume_.fade());
            }
            break;
        case AudioEvent::VOLUME_DEC_CMD:
            if (volume_.volume() > kVolumeMin) {
                sendVolumeSetCmd(yield, volume_.volume() - 1, volume_.fade());
            } else {
                sendVolumeSetCmd(yield, volume_.volume(), volume_.fade());
            }
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

        // Balance commands.
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

        // Fade commands.
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

        // Equalizer commands.
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

        // Stateless playback commands.
        case AudioEvent::PLAYBACK_TOGGLE_CMD:
            handlePlaybackToggleCmd(yield);
            break;
        case AudioEvent::PLAYBACK_NEXT_CMD:
            handlePlaybackNextCmd(yield);
            break;
        case AudioEvent::PLAYBACK_PREV_CMD:
            handlePlaybackPrevCmd(yield);
            break;

        // Settings commands.
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
                yield(MessageView(&settings_exit_));
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

    // Detect state.
    if (msg.pgn() == 0x1F014) {
        state_ = FusionState::PGN_01F014;
        state_pgn_ = msg.pgn();
        state_counter_ = counter(msg);
    } else if (msg.pgn() == 0x1F016) {
        state_ = FusionState::PGN_01F016;
        state_pgn_ = msg.pgn();
        state_counter_ = counter(msg);
    } else if (msg.pgn() == 0x1FF04 && msg.source_address() == hu_address_) {
        if (counter_seq(msg) == 0) {
            if ((msg.pgn() != state_pgn_ || counter_id(msg) != counter_id(state_counter_)) &&
                    match(msg.data() + 2, {0xA3, 0x99, 0xFF, 0x80})) {
                // first state message
                state_ = (FusionState)msg.data()[4];
            } else {
                state_ = FusionState::INVALID;
            }
        } else if (counter_seq(msg) != counter_seq(state_counter_) + 1) {
            // message out of sequence
            state_ = FusionState::INVALID;
        } else if (state_ignore_next_) {
            // ignore requested
            state_ignore_next_ = false;
            state_ = FusionState::INVALID;
        }
        // else use current state 
        state_pgn_ = msg.pgn();
        state_counter_ = counter(msg);
    } else {
        return;
    }

    // Handle state message.
    uint8_t seq = counter_seq(msg);
    switch (state_) {
        // System state messages.
        case PGN_01F014:
            handlePGN01F014(seq, msg, yield);
            break;
        case PGN_01F016:
            handlePGN01F016(seq, msg, yield);
            break;
        case POWER:
            handlePower(seq, msg, yield);
            break;
        case HEARTBEAT:
            handleHeartbeat(seq, msg, yield);
            break;

        // Volume and equalizer messages.
        case VOLUME:
            handleVolume(seq, msg, yield);
            break;
        case MUTE:
            handleMute(seq, msg, yield);
            break;
        case BALANCE:
            handleBalance(seq, msg, yield);
            break;
        case TONE:
            handleTone(seq, msg, yield);
            break;

        // Source messages.
        case SOURCE:
            handleSource(seq, msg, yield);
            break;
        case TRACK_PLAYBACK:
            handleTrackPlayback(seq, msg, yield);
            break;
        case TRACK_TITLE:
            handleTrackString(seq, msg, &track_title_, yield);
            break;
        case TRACK_ARTIST:
            handleTrackString(seq, msg, &track_artist_, yield);
            break;
        case TRACK_ALBUM:
            handleTrackString(seq, msg, &track_album_, yield);
            break;
        case TRACK_ELAPSED:
            handleTrackTimeElapsed(seq, msg, yield);
            break;
        case RADIO_FREQUENCY:
            handleRadioFrequency(seq, msg, yield);
            break;
        case INPUT_GAIN:
            handleInputGain(seq, msg, yield);
            break;

        // Settings menu messages.
        case MENU_LOAD:
            handleMenuLoad(seq, msg, yield);
            break;
        case MENU_ITEM_COUNT:
            handleMenuItemCount(seq, msg, yield);
            break;
        case MENU_ITEM_LIST:
            handleMenuItemList(seq, msg, yield);
            break;
    }
}

void Fusion::handlePGN01F014(uint8_t seq, const Canny::J1939Message& msg,
        const Yield<Message>& yield) {
    // 19F0140A#A0:86:35:08:8E:12:4D:53
    if (seq == 0) {
        // first message triggers boot init
        bootInit(msg.source_address(), yield);
    }
    if (msg.data()[7] == 0xFF && boot_state_ == DISCOVERED) {
        // last message triggers boot announce
        bootAnnounce(yield);
    }
}

void Fusion::handlePGN01F016(uint8_t, const Canny::J1939Message& msg,
        const Yield<Message>& yield) {
    if (msg.data()[7] == 0xFF && boot_state_ == ANNOUNCED) {
        // last message triggers boot request
        bootRequest(yield);
    }
}

void Fusion::handlePower(uint8_t seq, const Canny::J1939Message& msg,
        const Yield<Message>& yield) {
    if (seq != 0) {
        return;
    }
    updatePower(msg.data()[6] != 0x00, yield);
}

void Fusion::handleHeartbeat(uint8_t seq, const Canny::J1939Message& msg,
        const Yield<Message>& yield) {
    if (seq != 0) {
        return;
    }
    updatePower(msg.data()[6] != 0x02, yield);
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
        if (zone1 != zone2 && volume_.fade() == 0) {
            // calculate the fade value if we don't have one stored
            changed |= volume_.fade((zone1 - zone2) / kFadeMultiplier);
        }
        if (changed) {
            yield(MessageView(&volume_));
        }
    }
}

void Fusion::handleMute(uint8_t seq, const J1939Message& msg,
        const Yield<Message>& yield) {
    if (seq == 0 && volume_.mute(msg.data()[6] == 0x01) && system_.state() == AudioSystem::ON)  {
        yield(MessageView(&volume_));
    }
}

void Fusion::handleBalance(uint8_t seq, const J1939Message& msg,
        const Yield<Message>& yield) {
    // We balance all zones together so we only need to read
    // balance from zone 1.
    if (seq == 0 && msg.data()[6] == 0x00 &&
            volume_.balance(msg.data()[7]) &&
            system_.state() == AudioSystem::ON) {
        yield(MessageView(&volume_));
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
                state_ignore_next_ = true;
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
    if (changed && system_.state() == AudioSystem::ON) {
        yield(MessageView(&tone_));
    }
}

void Fusion::handleSource(uint8_t seq, const J1939Message& msg,
        const Yield<Message>& yield) {
    switch (seq) {
        case 0:
            secondary_source_ = (AudioSource)msg.data()[6];
            if (msg.data()[6] == msg.data()[7]) {
                source_.source((AudioSource)msg.data()[7]);
            }
            break;
        case 1:
            if (source_.source() == secondary_source_) {
                if (source_.source() == AudioSource::BLUETOOTH) {
                    switch (msg.data()[2]) {
                        case 0xA5:
                            source_.bt_connected(true);
                            break;
                        case 0x85:
                            source_.bt_connected(false);
                            break;
                        default:
                            break;
                    }
                }
                if (system_.state() == AudioSystem::POWER_ON) {
                    system_.state(AudioSystem::ON);
                    bootComplete(yield);
                } else if (system_.state() == AudioSystem::ON) {
                    yield(MessageView(&source_));
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
            track_playback_.playback((AudioPlayback)msg.data()[7]);
            break;
        case 2:
            memcpy(buffer_, msg.data() + 3, 3);
            buffer_[3] = 0x00;
            time = btohl(buffer_, ByteOrder::LITTLE);
            track_playback_.time_total(time / 1000);
            if (system_.state() == AudioSystem::ON) {
                yield(MessageView(&track_playback_));
            }
            break;
        default:
            break;
    }
}

void Fusion::handleTrackString(uint8_t seq, const J1939Message& msg,
        Event* event, const Yield<Message>& yield) {
    if (handleString(event->scratch, seq, msg, 4)) {
        yield(MessageView(event));
    }
}

void Fusion::handleTrackTimeElapsed(uint8_t seq, const J1939Message& msg,
        const Yield<Message>& yield) {
    if (seq == 1) {
        memcpy(buffer_, msg.data() + 1, 3);
        buffer_[3] = 0x00;
        uint32_t time = btohl(buffer_, ByteOrder::LITTLE);
        track_playback_.time_elapsed(time / 4);
        if (system_.state() == AudioSystem::ON) {
            yield(MessageView(&track_playback_));
        }
    }
}

void Fusion::handleRadioFrequency(uint8_t seq, const J1939Message& msg,
        const Yield<Message>& yield) {
    if (seq == 1) {
        uint32_t frequency = btohl(msg.data() + 1, ByteOrder::LITTLE);
        radio_.frequency(frequency);
        if (system_.state() == AudioSystem::ON) {
            yield(MessageView(&radio_));
        }
    }
}

void Fusion::handleInputGain(uint8_t seq, const J1939Message& msg,
        const Yield<Message>& yield) {
    if (seq == 0) {
        input_.gain((int8_t)msg.data()[7]);
        if (system_.state() == AudioSystem::ON) {
            yield(MessageView(&input_));
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
                        yield(MessageView(&event));
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
        yield(MessageView(&settings_menu_));
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
        yield(MessageView(&settings_item_));
    }
}

void Fusion::handleSourceNextCmd(const Caster::Yield<Message>& yield) {
    switch (source_.source()) {
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
    switch (source_.source()) {
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
    switch (source_.source()) {
        case AudioSource::AM:
        case AudioSource::FM:
            radio_.toggle_seek_mode();
            yield(MessageView(&radio_));
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
    switch (source_.source()) {
        case AudioSource::AM:
        case AudioSource::FM:
            switch (radio_.seek_mode()) {
                case AudioSeek::AUTO:
                    sendRadioCmd(yield, RADIO_CMD_NEXT_AUTO, radio_.frequency());
                    break;
                case AudioSeek::MANUAL:
                    sendRadioCmd(yield, RADIO_CMD_NEXT_MANUAL, radio_.frequency());
                    break;
            }
            break;
        case AudioSource::BLUETOOTH:
            sendTrackCmd(yield, TRACK_CMD_NEXT);
            break;
        case AudioSource::AUX:
        case AudioSource::OPTICAL:
            sendInputGainSetCmd(yield, input_.gain() + 1);
            break;
        default:
            break;
    }
}

void Fusion::handlePlaybackPrevCmd(const Caster::Yield<Message>& yield) {
    switch (source_.source()) {
        case AudioSource::AM:
        case AudioSource::FM:
            switch (radio_.seek_mode()) {
                case AudioSeek::AUTO:
                    sendRadioCmd(yield, RADIO_CMD_PREV_AUTO, radio_.frequency());
                    break;
                case AudioSeek::MANUAL:
                    sendRadioCmd(yield, RADIO_CMD_PREV_MANUAL, radio_.frequency());
                    break;
            }
            break;
        case AudioSource::BLUETOOTH:
            sendTrackCmd(yield, TRACK_CMD_PREV);
            break;
        case AudioSource::AUX:
        case AudioSource::OPTICAL:
            sendInputGainSetCmd(yield, input_.gain() - 1);
            break;
        default:
            break;
    }
}

bool Fusion::handleString(Scratch* scratch, uint8_t seq, const J1939Message& msg, uint8_t offset) {
    if (seq == 0) {
        scratch->clear();
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
        return;
    }

    if (boot_timer_.active()) {
        if (boot_state_ == DISCOVERED) {
            bootAnnounce(yield);
        } else if (boot_state_ == ANNOUNCED) {
            bootRequest(yield);
        } else if (boot_state_ == REQUESTED || boot_state_ == POWERING) {
            system_.state(AudioSystem::OFF);
            bootComplete(yield);
        }
    }
}

void Fusion::sendStereoRequest(const Yield<Message>& yield) {
    // 1DEF0A10#A0:04:A3:99:01:00:FF:FF
    sendCmd(yield, 0x04, 0x01);
}

void Fusion::send0CRequest(const Yield<Message>& yield) {
    // 1DEF0A20#40:08:A3:99:0C:00:39:24
    // 1DEF0A20#41:00:00:FF:FF:FF:FF:FF
    sendCmd(yield, 0x08, 0x0C, 0x39, 0x24);
    sendCmdPayload(yield, {0x00, 0x00});
}

void Fusion::sendStereoDiscovery(const Caster::Yield<Message>& yield) {
    disco_timer_.reset();
    sendPGN01F014Request(yield);
}

void Fusion::sendPGN01F014Request(const Yield<Message>& yield) {
    // 18EAFF21#14:F0:01
    J1939Message msg(0xEAFF, address_, 0xFF, 0x06);
    msg.data({0x14, 0xF0, 0x01});
    yield(MessageView(&msg));
}

void Fusion::sendPGN01F016Request(const Yield<Message>& yield) {
    // 18EAFF21#16:F0:01
    J1939Message msg(0xEAFF, address_, 0xFF, 0x06);
    msg.data({0x16, 0xF0, 0x01});
    yield(MessageView(&msg));
}

void Fusion::sendCmd(const Yield<Message>& yield,
        uint8_t cs, uint8_t id, uint8_t payload0, uint8_t payload1) {
    cmd_counter_ = (cmd_counter_ & 0xE0) + 0x20;
    cmd_.data({cmd_counter_++, cs, 0xA3, 0x99, id, 0x00, payload0, payload1});
    yield(MessageView(&cmd_));
}

void Fusion::sendCmdPayload(const Yield<Message>& yield, uint32_t payload) {
    cmd_.data()[0] = cmd_counter_++;
    // Fusion expects ints in little-endian byte order.
    ByteOrder::hltob(cmd_.data() + 1, payload, ByteOrder::LITTLE);
    memset(cmd_.data() + 5, 0xFF, 3);
    yield(MessageView(&cmd_));
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
    sendCmd(yield, 0x0A, 0x05, (uint8_t)source_.source(), cmd);
    sendCmdPayload(yield, freq);
}

void Fusion::sendInputGainSetCmd(const Yield<Message>& yield, int8_t gain) {
    // 1DEF0A21#60:06:A3:99:0D:00:03:01
    //                             |  |
    //                             |  +- gain (signed)
    //                             +---- source
    sendCmd(yield, 0x06, 0x0D, (uint8_t)source_.source(), gain);
}

void Fusion::sendVolumeSetCmd(const Yield<Message>& yield, uint8_t volume, int8_t fade) {
    // 1DEF0A21#80:08:A3:99:19:00:11:0E
    //                             |  |
    //                             |  +- zone 2
    //                             +---- zone 1
    // 1DEF0A21#81:0A:01:FF:FF:FF:FF:FF
    //              |
    //              +------------------- zone 3

    clamp(&fade, kFadeMin, kFadeMax);
    volume_.fade(fade);
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
    yield(MessageView(&volume_));
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
    sendCmd(yield, 0x0B, 0x09, (uint8_t)source_.source(), item);
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
    sendCmd(yield, 0x06, 0x0A, (uint8_t)source_.source(), 0x03);
}

void Fusion::sendMenuReqItemList(const Yield<Message>& yield, uint8_t count) {
    // 1DEF0A21#60:0E:A3:99:0B:00:07:00
    // 1DEF0A21#61:00:00:00:03:00:00:00
    //                       |
    //                       +---------- item count
    // 1DEF0A21#82:03:FF:FF:FF:FF:FF:FF
    sendCmd(yield, 0x0E, 0x0B, (uint8_t)source_.source(), 0x00);
    sendCmdPayload(yield, {0x00, 0x00, 0x00, count, 0x00, 0x00, 0x00});
    sendCmdPayload(yield, {0x03});
}

void Fusion::updatePower(bool power, const Yield<Message>& yield) {
    AudioSystem state = power ? AudioSystem::ON : AudioSystem::OFF;
    if (system_.state() == AudioSystem::UNAVAILABLE) {
        // Rely on discovery to transition from unavailable as doing so
        // here would put us into an invalid state.
        sendStereoDiscovery(yield);
    } else if (system_.state() != state) {
        bootPower(power, yield);
    }
}

// Reset internal state and start the boot process.
void Fusion::bootInit(uint8_t hu_address, const Yield<Message>& yield) {
    // configure head unit address
    hu_address_ = hu_address;
    cmd_.dest_address(hu_address);

    // disable discovery timer
    disco_timer_.pause();

    // set system state to booting
    boot_state_ = DISCOVERED;
    boot_timer_.resume(kBootInitTimeout);
    system_.state(AudioSystem::BOOT);
    yield(MessageView(&system_));
}

// Send PGN request for system state and wait for boot to complete.
void Fusion::bootAnnounce(const Yield<Message>& yield) {
    // send PGN 01F016 request to get system state
    sendPGN01F016Request(yield);
    boot_state_ = ANNOUNCED;
    boot_timer_.reset();
}

// Request source and track information.
void Fusion::bootRequest(const Yield<Message>& yield) {
    boot_state_ = REQUESTED;
    boot_timer_.reset(kBootRequestTimeout);
    sendStereoRequest(yield);
    send0CRequest(yield);
}

void Fusion::bootPower(bool power, const Caster::Yield<Message>& yield) {
    if (power && system_.state() != AudioSystem::POWER_ON) {
        boot_timer_.reset();
        system_.state(AudioSystem::POWER_ON);
        yield(MessageView(&system_));
        if (boot_state_ != REQUESTED) {
            sendStereoRequest(yield);
            send0CRequest(yield);
        }
        boot_state_ = POWERING;
    } else if (!power) {
        if (system_.state() == AudioSystem::BOOT) {
            system_.state(AudioSystem::OFF);
            bootComplete(yield);
        } else {
            system_.state(AudioSystem::OFF);
            yield(MessageView(&system_));
        }
    }
}

// Send all state events and exit boot.
void Fusion::bootComplete(const Yield<Message>& yield) {
    // disable boot mode
    boot_state_ = UNKNOWN;
    boot_timer_.pause();

    // send all state events
    yield(MessageView(&volume_));
    yield(MessageView(&tone_));
    yield(MessageView(&source_));
    switch (source_.source()) {
        case AudioSource::AM:
        case AudioSource::FM:
            yield(MessageView(&radio_));
            break;
        case AudioSource::AUX:
        case AudioSource::OPTICAL:
            yield(MessageView(&input_));
            break;
        case AudioSource::BLUETOOTH:
            yield(MessageView(&track_playback_));
            yield(MessageView(&track_title_));
            yield(MessageView(&track_artist_));
            yield(MessageView(&track_album_));
            break;
        default:
            break;
    }
    yield(MessageView(&system_));
}

}  // namespace R51
