#ifndef _R51_CONTROLS_HMI_H_
#define _R51_CONTROLS_HMI_H_

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>
#include <Vehicle.h>
#include "Audio.h"
#include "Controls.h"
#include "HMIEvent.h"

namespace R51 {

// Node for interacting with an attached HMI LED display.
class HMI : public Controls {
    public:
        // Construct a new HMI node that communicates with a device over the
        // given stream. The scratch space is used to provide string data to
        // the display for events which include a string payload. 
        HMI(Stream* stream, Scratch* scratch, uint8_t encoder_keypad_id = 0xFF);

        // Updates the HMI display with received broadcast events.
        void handle(const Message& msg, const Caster::Yield<Message>&) override;

        // Emit input events from the HMI display.
        void emit(const Caster::Yield<Message>& yield) override;
    private:
        void handleECM(const Event& event);
        void handleIPDM(const Event& event);
        void handleTire(const Event& event);
        void handlePowerState(const PowerState* power);
        void handleClimateSystem(const ClimateSystemState* event);
        void handleClimateAirflow(const ClimateAirflowState* event);
        void handleClimateTemp(const ClimateTempState* event);
        void handleSettings(const Event& event);
        void handleAudioSystem(const AudioSystemState* event);
        void handleAudioVolume(const AudioVolumeState* event);
        void handleAudioTone(const AudioToneState* event);
        void handleAudioPlayback(const AudioTrackPlaybackState* event);
        void handleAudioSettingsMenu(const AudioSettingsMenuState* event);
        void handleAudioSettingsItem(const AudioSettingsItemState* event);
        void handleAudioSettingsExit(const AudioSettingsExitState* event);

        void handleSerial(const Caster::Yield<Message>& yield);
        void handleClimateButton(uint8_t button, const Caster::Yield<Message>& yield);
        void handleAudioRadioButton(uint8_t button, const Caster::Yield<Message>& yield);
        void handleAudioTrackButton(uint8_t button, const Caster::Yield<Message>& yield);
        void handleAudioSourceButton(uint8_t button, const Caster::Yield<Message>& yield);
        void handleAudioSettingsButton(uint8_t button, const Caster::Yield<Message>& yield);
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
        bool isPage(HMIPage value);
        bool isAudioPage();
        bool isAudioSourcePage();
        bool isSettingsPage();
        bool isPageWithHeader();
        void page(HMIPage value);
        void climatePopup();
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
        void setAudioSettingsItem(uint8_t item, uint8_t type);

        bool read(bool block);

        Stream* stream_;
        Scratch* scratch_;
        uint8_t encoder_keypad_id_;

        DisplayPageState page_;
        DisplaySleepState sleep_;

        // climate state
        ClimateSystemMode climate_system_;
        uint8_t climate_fan_;
        uint8_t climate_driver_temp_;
        uint8_t climate_pass_temp_;

        // audio state
        bool mute_;
        bool audio_available_;
        uint8_t audio_settings_page_;
        uint8_t audio_settings_count_;
};

}  // namespace R51

#endif  // _R51_CONTROLS_HMI_H_
