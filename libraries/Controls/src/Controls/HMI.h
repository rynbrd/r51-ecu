#ifndef _R51_CONTROLS_HMI_H_
#define _R51_CONTROLS_HMI_H_

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>
#include <Vehicle.h>
#include "Audio.h"
#include "Controls.h"
#include "Screen.h"

namespace R51 {

// Node for interacting with an attached HMI LED display.
class HMI : public Controls {
    public:
        // Construct a new HMI node that communicates with a device over the
        // given stream.
        HMI(Stream* stream, uint8_t encoder_keypad_id = 0xFF,
                uint8_t pdm_id = 0xFF);

        // Initialize display state. 
        void init(const Caster::Yield<Message>&) override;

        // Updates the HMI display with received broadcast events.
        void handle(const Message& msg, const Caster::Yield<Message>&) override;

        // Emit input events from the HMI display.
        void emit(const Caster::Yield<Message>& yield) override;
    private:
        void handleECM(const Event& event);
        void handleIPDM(const Event& event);
        void handleIllum(const Caster::Yield<Message>& yield, const Event& event);
        void handleTire(const Event& event);
        void handlePowerState(const PowerState* power);
        void handleClimateSystem(const ClimateSystemState* event);
        void handleClimateAirflow(const ClimateAirflowState* event);
        void handleClimateTemp(const ClimateTempState* event);
        void handleSettings(const Event& event);
        void handleAudioSystem(const AudioSystemState* event);
        void handleAudioVolume(const AudioVolumeState* event);
        void handleAudioTone(const AudioToneState* event);
        void handleAudioSource(const AudioSourceState* event);
        void handleAudioPlayback(const AudioTrackPlaybackState* event);
        void handleAudioRadio(const AudioRadioState* event);
        void handleAudioInput(const AudioInputState* event);
        void handleAudioSettingsMenu(const AudioSettingsMenuState* event);
        void handleAudioSettingsItem(const AudioSettingsItemState* event);
        void handleAudioSettingsExit(const AudioSettingsExitState* event);

        void handleSerial(const Caster::Yield<Message>& yield);
        void handleClimateButton(uint8_t button, const Caster::Yield<Message>& yield);
        void handleAudioRadioButton(uint8_t button, const Caster::Yield<Message>& yield);
        void handleAudioTrackButton(uint8_t button, const Caster::Yield<Message>& yield);
        void handleAudioAuxButton(uint8_t button, const Caster::Yield<Message>& yield);
        void handleAudioSourceButton(uint8_t button, const Caster::Yield<Message>& yield);
        void handleAudioSettingsButton(uint8_t button, const Caster::Yield<Message>& yield);
        void handleAudioEqButton(uint8_t button, const Caster::Yield<Message>& yield);
        void handleAudioPowerOffButton(uint8_t button, const Caster::Yield<Message>& yield);
        void handleVehicleButton(uint8_t button, const Caster::Yield<Message>& yield);
        void handleSettings1Button(uint8_t button, const Caster::Yield<Message>& yield);
        void handleSettings2Button(uint8_t button, const Caster::Yield<Message>& yield);
        void handleSettings3Button(uint8_t button, const Caster::Yield<Message>& yield);

        void navUp(const Caster::Yield<Message>& yield);
        void navDown(const Caster::Yield<Message>& yield);
        void navLeft(const Caster::Yield<Message>& yield);
        void navRight(const Caster::Yield<Message>& yield);
        void navActivate(const Caster::Yield<Message>& yield);
        void navPageNext(const Caster::Yield<Message>& yield);
        void navPagePrev(const Caster::Yield<Message>& yield);

        void terminate();
        void refresh();
        void back();
        void show(const char* obj);
        void hide(const char* obj); 
        void power(bool power);
        void brightness(const Caster::Yield<Message>& yield, uint8_t brightness);
        bool isPage(ScreenPage value);
        bool isAudioPage();
        bool isAudioSourcePage();
        bool isSettingsPage();
        bool isPageWithHeader();
        void page(ScreenPage value);
        void maybeClimatePopup();
        void printEscaped(const char* value);
        int32_t getVal(const char* key);
        void setVal(const char* key, int32_t value);
        void setTxt(const char* key, int32_t value);
        void setTxt(const char* key, double value, uint8_t precision);
        void setTxt(const char* key, const char* value);
        void setTxt(const char* key, Scratch* scratch);
        void setTxtTemp(const char* key, int32_t degrees);
        void setTxtTime(const char* key, uint16_t seconds);
        void setVolume(uint8_t value);
        void setGain(int8_t db);
        void setAudioSettingsItem(uint8_t item, uint8_t type, Scratch* scratch);

        bool read(bool block);

        Stream* stream_;
        Scratch scratch_;
        uint8_t encoder_keypad_id_;
        uint8_t pdm_id_;

        ScreenPageState page_;
        ScreenPowerState power_;

        // climate state
        ClimateSystemMode climate_system_;
        uint8_t climate_fan_;
        uint8_t climate_driver_temp_;
        uint8_t climate_pass_temp_;
        uint8_t climate_out_temp_;

        // audio state
        bool mute_;
        uint8_t volume_;
        AudioSystem audio_system_;
        AudioSource audio_source_;
        uint8_t audio_settings_page_;
        uint8_t audio_settings_count_;
};

}  // namespace R51

#endif  // _R51_CONTROLS_HMI_H_
