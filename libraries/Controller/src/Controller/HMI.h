#ifndef _R51_CONTROLLER_HMI_H_
#define _R51_CONTROLLER_HMI_H_

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>
#include <Vehicle.h>
#include "Fusion.h"

namespace R51 {

enum class Page : uint8_t {
    SPLASH = 0,
    HOME = 1,
    CLIMATE = 2,
    AUDIO = 3,
    AUDIO_TRACK = 4,
    AUDIO_RADIO = 5,
    AUDIO_AUX = 6,
    AUDIO_NO_STEREO = 7,
    AUDIO_VOLUME = 8,
    AUDIO_SOURCE = 9,
    AUDIO_BT = 10,
    AUDIO_CONNECT = 11,
    AUDIO_FORGET = 12,
    VEHICLE = 13,
    SETTINGS = 14,
    SETTINGS_1 = 15,
    SETTINGS_2 = 16,
    SETTINGS_3 = 17,
    SHARED = 18,
};

enum class HMIEvent : uint8_t {
    PAGE_STATE = 0x01,  // State event. The current page.
    SLEEP_STATE = 0x02, // State event. Sent when the display sleeps and wakes.
};

class DisplayPageState : public Event {
    public:
        DisplayPageState() :
            Event(SubSystem::HMI, (uint8_t)HMIEvent::PAGE_STATE, {0x00}) {}

        EVENT_PROPERTY(uint8_t, page, data[0], data[0] = value);
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
class HMI : public Caster::Node<Message> {
    public:
        // Construct a new HMI node that communicates with a device over the
        // given stream. The scratch space is used to provide string data to
        // the display for events which include a string payload. 
        HMI(Stream* stream, Scratch* scratch);

        // Updates the HMI display with received broadcast events.
        void handle(const Message& msg, const Caster::Yield<Message>&) override;

        // Emit input events from the HMI display.
        void emit(const Caster::Yield<Message>& yield) override;
    private:
        void handleECM(const Event& event);
        void handleIPDM(const Event& event);
        void handleTire(const Event& event);
        void handleClimateSystem(const ClimateSystemStateEvent* event);
        void handleClimateAirflow(const ClimateAirflowStateEvent* event);
        void handleClimateTemp(const ClimateTempStateEvent* event);
        void handleSettings(const Event& event);
        void handleAudioSystem(const AudioSystemState* event);
        void handleAudioVolume(const AudioVolumeState* event);
        void handleAudioMute(const AudioMuteState* event);
        void handleAudioTone(const AudioToneState* event);
        void handleAudioPlayback(const AudioTrackPlaybackState* event);

        void handleSerial(const Caster::Yield<Message>& yield);
        void handleClimateButton(uint8_t button, const Caster::Yield<Message>& yield);
        void handleAudioRadioButton(uint8_t button, const Caster::Yield<Message>& yield);
        void handleAudioSourceButton(uint8_t button, const Caster::Yield<Message>& yield);
        void handleAudioBtButton(uint8_t button, const Caster::Yield<Message>& yield);
        void handleAudioConnectButton(uint8_t button, const Caster::Yield<Message>& yield);
        void handleAudioForgetButton(uint8_t button, const Caster::Yield<Message>& yield);
        void handleVehicleButton(uint8_t button, const Caster::Yield<Message>& yield);
        void handleSettings1Button(uint8_t button, const Caster::Yield<Message>& yield);
        void handleSettings2Button(uint8_t button, const Caster::Yield<Message>& yield);
        void handleSettings3Button(uint8_t button, const Caster::Yield<Message>& yield);

        void terminate();
        void refresh();
        void show(const char* obj);
        void hide(const char* obj); 
        bool isPage(Page value);
        bool isAudioSourcePage();
        void page(Page value);
        void printEscaped(const char* value);
        int32_t get(const char* key);
        void setVal(const char* key, int32_t value);
        void setTxt(const char* key, int32_t value);
        void setTxt(const char* key, double value, uint8_t precision);
        void setTxt(const char* key, const char* value);
        void setTxt(const char* key, Scratch* scratch);
        void setTxtTemp(const char* key, int32_t degrees);
        void setTxtTime(const char* key, uint16_t seconds);
        void setVolume(uint8_t value);
        void setGain(int8_t db);

        bool read(bool block);

        DisplayPageState page_;
        DisplaySleepState sleep_;

        Stream* stream_;
        Scratch* scratch_;
        uint8_t climate_system_;
};

}  // namespace R51

#endif  // _R51_CONTROLLER_HMI_H_
