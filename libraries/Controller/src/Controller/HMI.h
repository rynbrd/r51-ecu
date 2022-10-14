#ifndef _R51_CONTROLLER_HMI_H_
#define _R51_CONTROLLER_HMI_H_

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>
#include <Faker.h>
#include <Vehicle.h>
#include "Audio.h"
#include "Controls.h"

namespace R51 {

enum class HMIPage : uint8_t {
    SPLASH          = 0,
    HOME            = 1,
    CLIMATE         = 2,
    AUDIO           = 3,
    AUDIO_TRACK     = 4,
    AUDIO_RADIO     = 5,
    AUDIO_AUX       = 6,
    AUDIO_POWER_OFF = 7,
    AUDIO_NO_STEREO = 8,
    AUDIO_VOLUME    = 9,
    AUDIO_SOURCE    = 10,
    AUDIO_SETTINGS  = 11,
    AUDIO_EQ        = 12,
    VEHICLE         = 13,
    SETTINGS        = 14,
    SETTINGS_1      = 15,
    SETTINGS_2      = 16,
    SETTINGS_3      = 17,
    SHARED          = 18,
};

enum class HMIEvent : uint8_t {
    PAGE_STATE  = 0x00, // State event. The current page.
    SLEEP_STATE = 0x01, // State event. Sent when the display sleeps and wakes.

    // Context sensitive navigation commands.
    NAV_UP_CMD          = 0x10,
    NAV_DOWN_CMD        = 0x11,
    NAV_LEFT_CMD        = 0x12,
    NAV_RIGHT_CMD       = 0x13,
    NAV_ACTIVATE_CMD    = 0x14,
};

class DisplayPageState : public Event {
    public:
        DisplayPageState() :
            Event(SubSystem::HMI, (uint8_t)HMIEvent::PAGE_STATE, {0x00}) {}

        EVENT_PROPERTY(HMIPage, page, (HMIPage)data[0], data[0] = (uint8_t)value);
};

class DisplaySleepState : public Event {
    public:
        DisplaySleepState() :
            Event(SubSystem::HMI, (uint8_t)HMIEvent::PAGE_STATE, {0x00}) {}

        EVENT_PROPERTY(bool, sleep, data[0] == 0x01, data[0] = (uint8_t)value);
};

class HMIDebugStream : public Stream {
    public:
        HMIDebugStream(Stream* child) : child_(child), n_(0) {}

        int available() override {
            return child_->available();
        }

        int read() override {
            return child_->read();
        }

        int peek() override {
            return child_->peek();
        }

        size_t write(uint8_t b) override {
            if (b == 0xFF && n_ > 0) {
                Serial.println();
                n_ = 0;
            } else if (b != 0xFF) {
                if (n_ == 0) {
                    Serial.print("hmi send: ");
                }
                Serial.write(b);
                ++n_;
            }
            return child_->write(b);
        }

    private:
        Stream* child_;
        int n_;
};

// Node for interacting with an attached HMI LED display.
class HMI : public Controls {
    public:
        // Construct a new HMI node that communicates with a device over the
        // given stream. The scratch space is used to provide string data to
        // the display for events which include a string payload. 
        HMI(Stream* stream, Scratch* scratch, uint8_t encoder_keypad_id = 0xFF,
                Faker::Clock* clock = Faker::Clock::real());

        // Updates the HMI display with received broadcast events.
        void handle(const Message& msg, const Caster::Yield<Message>&) override;

        // Emit input events from the HMI display.
        void emit(const Caster::Yield<Message>& yield) override;
    private:
        void handleEncoder(const Event& event, const Caster::Yield<Message>& yield);
        void handleEncoderClimate(const Event& event, const Caster::Yield<Message>& yield);
        void handleEncoderAudio(const Event& event, const Caster::Yield<Message>& yield);
        void handleEncoderNav(const Event& event, const Caster::Yield<Message>& yield);

        void handleECM(const Event& event);
        void handleIPDM(const Event& event);
        void handleTire(const Event& event);
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

        void terminate();
        void refresh();
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
        uint8_t audio_settings_page_;
        uint8_t audio_settings_count_;

        // buttons
        LongPressButton power_;
};

}  // namespace R51

#endif  // _R51_CONTROLLER_HMI_H_
