#ifndef _R51_CONTROLS_FUSION_H_
#define _R51_CONTROLS_FUSION_H_

#include <Arduino.h>
#include <ByteOrder.h>
#include <CRC32.h>
#include <Caster.h>
#include <Core.h>
#include <Faker.h>
#include <Foundation.h>
#include "Audio.h"

namespace R51 {

// Node for interacting with Garming Fusion head units over J1939/NMEA2000.
class Fusion : public Caster::Node<Message> {
    public:
        // Construct a fusion node.
        Fusion(Faker::Clock* clock = Faker::Clock::real());

        // Handle J1939 state messages from the head unit and control Events
        // from other devices.
        void handle(const Message& msg, const Caster::Yield<Message>& yield) override;

        // Emit state events from the head units.
        void emit(const Caster::Yield<Message>& yield) override;

    private:
        // Handle commands from the internal bus.
        void handleCommand(const Event& event, const Caster::Yield<Message>& yield);

        // Handle J1939 messages.
        void handleJ1939Claim(const J1939Claim& claim,
                const Caster::Yield<Message>& yield);
        void handleJ1939Message(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);

        // Handle Fusion J1939 state messages. 
        void handlePGN01F014(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handlePGN01F016(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handlePower(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleHeartbeat(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);

        void handleVolume(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleMute(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleBalance(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleTone(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);

        void handleSource(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleTrackPlayback(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleTrackString(uint8_t seq,
                const Canny::J1939Message& msg, Event* event,
                const Caster::Yield<Message>& yield);
        void handleTrackTimeElapsed(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);

        void handleRadioFrequency(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);

        void handleInputGain(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);

        void handleMenuLoad(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleMenuItemCount(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);
        void handleMenuItemList(uint8_t seq, const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);

        void handleSourceNextCmd(const Caster::Yield<Message>& yield);
        void handleSourcePrevCmd(const Caster::Yield<Message>& yield);

        void handlePlaybackToggleCmd(const Caster::Yield<Message>& yield);
        void handlePlaybackNextCmd(const Caster::Yield<Message>& yield);
        void handlePlaybackPrevCmd(const Caster::Yield<Message>& yield);

        bool handleString(Scratch* scratch, uint8_t seq,
                const Canny::J1939Message& msg, uint8_t offset);

        void sendStereoRequest(const Caster::Yield<Message>& yield);
        void sendStereoDiscovery(const Caster::Yield<Message>& yield);
        void send0CRequest(const Caster::Yield<Message>& yield);

        void sendPGN01F014Request(const Caster::Yield<Message>& yield);
        void sendPGN01F016Request(const Caster::Yield<Message>& yield);

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

        void bootInit(uint8_t hu_address, const Caster::Yield<Message>& yield);
        void bootAnnounce(const Caster::Yield<Message>& yield);
        void bootRequest(const Caster::Yield<Message>& yield);
        void bootPower(bool power, const Caster::Yield<Message>& yield);
        void bootComplete(const Caster::Yield<Message>& yield);
        void updatePower(bool power, const Caster::Yield<Message>& yield);

        Faker::Clock* clock_;

        uint8_t address_;
        uint8_t hu_address_;
        uint8_t boot_state_;
        Ticker disco_timer_;
        Ticker boot_timer_;

        Scratch track_title_scratch_;
        Scratch track_artist_scratch_;
        Scratch track_album_scratch_;
        Scratch settings_item_scratch_;

        AudioSystemState system_;
        AudioVolumeState volume_;
        AudioToneState tone_;
        AudioSourceState source_;
        AudioTrackPlaybackState track_playback_;
        AudioTrackTitleState track_title_;
        AudioTrackArtistState track_artist_;
        AudioTrackAlbumState track_album_;
        AudioRadioState radio_;
        AudioInputState input_;
        AudioSettingsMenuState settings_menu_;
        AudioSettingsItemState settings_item_;
        AudioSettingsExitState settings_exit_;

        uint8_t state_;
        bool state_ignore_next_;
        uint32_t state_pgn_;
        uint8_t state_counter_;
        uint8_t cmd_counter_;
        Canny::J1939Message cmd_;

        AudioSource secondary_source_;

        uint8_t buffer_[4];
};

template <size_t N> 
void Fusion::sendCmdPayload(const Caster::Yield<Message>& yield, const uint8_t (&payload)[N]) {
    uint8_t i;
    cmd_.data()[0] = cmd_counter_++;
    for (i = 0; i < N; ++i) {
        cmd_.data()[i + 1] = payload[i];
    }
    memset(cmd_.data() + N + 1, 0xFF, 7 - N);
    yield(MessageView(&cmd_));
}

}  // namespace R51

#endif  // _R51_CONTROLS_FUSION_H_
