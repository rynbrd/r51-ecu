#include "HMI.h"

#include <Arduino.h>
#include <Bluetooth.h>
#include <Caster.h>
#include <Common.h>
#include <Endian.h>
#include <Faker.h>
#include <Foundation.h>
#include <Vehicle.h>
#include "Audio.h"
#include "HMIEvent.h"

namespace R51 {
namespace {

using ::Caster::Yield;

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


}  // namespace

HMI::HMI(Stream* stream, Scratch* scratch, uint8_t encoder_keypad_id,
        Faker::Clock* clock) :
    stream_(new HMIDebugStream(stream)), scratch_(scratch),
    encoder_keypad_id_(encoder_keypad_id),
    climate_system_(CLIMATE_SYSTEM_OFF), climate_fan_(0xFF),
    climate_driver_temp_(0xFF), climate_pass_temp_(0xFF),
    mute_(false), audio_settings_page_(0), audio_settings_count_(0) {}

void HMI::handle(const Message& msg, const Yield<Message>& yield) {
    if (msg.type() != Message::EVENT) {
        return;
    }
    const auto& event = msg.event();
    switch ((SubSystem)event.subsystem) {
        case SubSystem::CONTROLLER:
            if (RequestCommand::match(event, SubSystem::HMI, (uint8_t)HMIEvent::PAGE_STATE)) {
                yield(page_);
            }
            if (RequestCommand::match(event, SubSystem::HMI, (uint8_t)HMIEvent::SLEEP_STATE)) {
                yield(sleep_);
            }
            break;
        case SubSystem::HMI:
            switch ((HMIEvent)event.id) {
                case HMIEvent::NAV_UP_CMD:
                    navUp(yield);
                    break;
                case HMIEvent::NAV_DOWN_CMD:
                    navDown(yield);
                    break;
                case HMIEvent::NAV_LEFT_CMD:
                    navLeft(yield);
                    break;
                case HMIEvent::NAV_RIGHT_CMD:
                    navRight(yield);
                    break;
                case HMIEvent::NAV_ACTIVATE_CMD:
                    navActivate(yield);
                    break;
                default:
                    break;
            }
            break;
        case SubSystem::ECM:
            handleECM(event);
            break;
        case SubSystem::IPDM:
            handleIPDM(event);
            break;
        case SubSystem::TIRE:
            handleTire(event);
            break;
        case SubSystem::CLIMATE:
            switch ((ClimateEvent)event.id) {
                case ClimateEvent::SYSTEM_STATE:
                    handleClimateSystem((ClimateSystemState*)&event);;
                    break;
                case ClimateEvent::AIRFLOW_STATE:
                    handleClimateAirflow((ClimateAirflowState*)&event);
                    break;
                case ClimateEvent::TEMP_STATE:
                    handleClimateTemp((ClimateTempState*)&event);
                    break;
                default:
                    break;
            }
            break;
        case SubSystem::SETTINGS:
            handleSettings(msg.event());
            break;
        case SubSystem::AUDIO:
            switch ((AudioEvent)event.id) {
                case AudioEvent::SYSTEM_STATE:
                    handleAudioSystem((AudioSystemState*)&event);
                    break;
                case AudioEvent::VOLUME_STATE:
                    handleAudioVolume((AudioVolumeState*)&event);
                    break;
                case AudioEvent::TONE_STATE:
                    handleAudioTone((AudioToneState*)&event);
                    break;
                case AudioEvent::TRACK_PLAYBACK_STATE:
                    handleAudioPlayback((AudioTrackPlaybackState*)&event);
                    break;
                case AudioEvent::TRACK_TITLE_STATE:
                    setTxt("audio_track.title_txt", scratch_);
                    break;
                case AudioEvent::TRACK_ARTIST_STATE:
                    setTxt("audio_track.artist_txt", scratch_);
                    break;
                case AudioEvent::TRACK_ALBUM_STATE:
                    setTxt("audio_track.album_txt", scratch_);
                    break;
                case AudioEvent::SETTINGS_MENU_STATE:
                    handleAudioSettingsMenu((AudioSettingsMenuState*)&event);
                    break;
                case AudioEvent::SETTINGS_ITEM_STATE:
                    handleAudioSettingsItem((AudioSettingsItemState*)&event);
                    break;
                case AudioEvent::SETTINGS_EXIT_STATE:
                    handleAudioSettingsExit((AudioSettingsExitState*)&event);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void HMI::handleECM(const Event& event) {
    if (event.id != (uint8_t)ECMEvent::ENGINE_TEMP_STATE) {
        return;
    }
    int32_t value = event.data[0] - 40;
    if (value == 0) {
        setTxt("vehicle.etemp_txt", "");
        if (isPage(HMIPage::VEHICLE)) {
            hide("vehicle.etemp_label");
        }
    } else {
        setTxtTemp("vehicle.etemp_txt.txt=", value);
        if (isPage(HMIPage::VEHICLE)) {
            show("vehicle.etemp_label");
        }
    }
}

void HMI::handleIPDM(const Event& event) {
    if (event.id != (uint8_t)IPDMEvent::POWER_STATE) {
        return;
    }
    setVal("shared.fog_lamp", getBit(event.data, 0, 3));
    setVal("climate.defrost", getBit(event.data, 0, 6));
    refresh();
}

void HMI::handleTire(const Event& event) {
    if (event.id != (uint8_t)TireEvent::PRESSURE_STATE) {
        return;
    }
    if (event.data[0] == 0) {
        setTxt("vehicle.tire_fl_txt", "");
    } else {
        setTxt("vehicle.tire_fl_txt", event.data[0]);
    }
    if (event.data[1] == 0) {
        setTxt("vehicle.tire_fr_txt", "");
    } else {
        setTxt("vehicle.tire_fr_txt", event.data[1]);
    }
    if (event.data[2] == 0) {
        setTxt("vehicle.tire_rl_txt", "");
    } else {
        setTxt("vehicle.tire_rl_txt", event.data[2]);
    }
    if (event.data[3] == 0) {
        setTxt("vehicle.tire_rr_txt", "");
    } else {
        setTxt("vehicle.tire_rr_txt", event.data[3]);
    }
}

void HMI::handleClimateSystem(const ClimateSystemState* event) {
    climate_system_ = event->mode();
    setVal("climate.system", (uint8_t)climate_system_);
    setVal("climate.ac", event->ac());
    setVal("climate.dual", event->dual());
    if (isPage(HMIPage::CLIMATE)) {
        refresh();
    }
}

void HMI::handleClimateAirflow(const ClimateAirflowState* event) {
    setVal("climate.fan_bar", (int32_t)(100 * event->fan_speed() / 7));
    setVal("climate.vface", event->face());
    setVal("climate.vfeet", event->feet());
    setVal("climate.vdefog", event->windshield());
    setVal("climate.recirc", event->recirculate());
    if (isPage(HMIPage::CLIMATE)) {
        refresh();
    } else if (climate_fan_ != 0xFF && climate_fan_ != event->fan_speed() &&
            climate_system_ != CLIMATE_SYSTEM_AUTO) {
        climatePopup();
    }
    climate_fan_ = event->fan_speed();
}

void HMI::handleClimateTemp(const ClimateTempState* event) {
    setTxtTemp("climate.dtemp_txt", event->driver_temp());
    setTxtTemp("climate.ptemp_txt", event->passenger_temp());
    setTxtTemp("climate.otemp_txt", event->outside_temp());
    if ((climate_driver_temp_ != 0xFF && climate_driver_temp_ != event->driver_temp()) ||
            (climate_pass_temp_ != 0xFF && climate_pass_temp_ != event->passenger_temp())) {
        climatePopup();
    }
    climate_driver_temp_ = event->driver_temp();
    climate_pass_temp_ = event->passenger_temp();
}

void HMI::handleSettings(const Event& event) {
    if (event.id != (uint8_t)SettingsEvent::STATE) {
        return;
    }
    setVal("settings.illum", getBit(event.data, 0, 0));
    setVal("settings.autoseat", getBit(event.data, 0, 1));
    setVal("settings.wiper", getBit(event.data, 0, 2));
    setVal("settings.hlsens", event.data[1] & 0x03);
    setVal("settings.hlauto", ((event.data[1] << 2) & 0x03) * 15);
    setVal("settings.selunlock", getBit(event.data, 2, 0));
    setVal("settings.relock", (event.data[2] >> 4) & 0x0F);
    setVal("settings.keyhorn", getBit(event.data, 3, 0));
    setVal("settings.keylights", (event.data[3] >> 2) & 0x03);
    if (isPage(HMIPage::SETTINGS_1) || isPage(HMIPage::SETTINGS_2)) {
        refresh();
    }
}

void HMI::handleAudioSystem(const AudioSystemState* event) {
    setVal("audio.available", event->available());
    if (!event->available()) {
        if (isAudioPage() && !isPage(HMIPage::AUDIO_NO_STEREO)) {
            page(HMIPage::AUDIO_NO_STEREO);
        }
        return;
    }
    setVal("audio.power", event->power());
    if (!event->power()) {
        if (isAudioSourcePage() && !isPage(HMIPage::AUDIO_POWER_OFF)) {
            page(HMIPage::AUDIO_POWER_OFF);
        }
        return;
    }
    setVal("audio_radio.seek_mode", (uint8_t)event->seek_mode());
    if (isPage(HMIPage::AUDIO_RADIO)) {
        refresh();
    }

    setVal("audio.source", (uint8_t)event->source());
    switch (event->source()) {
        case AudioSource::AM:
            setTxt("audio_radio.freq_txt", (int32_t)(event->frequency() / 1000));
            setTxt("audio_radio.freq_label", "KHz");
            if (isAudioSourcePage() && !isPage(HMIPage::AUDIO_RADIO)) {
                page(HMIPage::AUDIO_RADIO);
            }
            break;
        case AudioSource::FM:
            setTxt("audio_radio.freq_txt", ((double)event->frequency() / 1000000.0), 1);
            setTxt("audio_radio.freq_label", "MHz");
            if (isAudioSourcePage() && !isPage(HMIPage::AUDIO_RADIO)) {
                page(HMIPage::AUDIO_RADIO);
            }
            break;
        case AudioSource::AUX:
            setTxt("audio_aux.input_txt", "Aux Input");
            setGain(event->gain());
            if (isAudioSourcePage() && !isPage(HMIPage::AUDIO_AUX)) {
                page(HMIPage::AUDIO_AUX);
            }
            break;
        case AudioSource::OPTICAL:
            setTxt("audio_aux.input_txt", "Optical Input");
            setGain(event->gain());
            if (isAudioSourcePage() && !isPage(HMIPage::AUDIO_AUX)) {
                page(HMIPage::AUDIO_AUX);
            }
            break;
        default:
            setVal("audio.device", event->bt_connected());
            if (isPageWithHeader() || isPage(HMIPage::AUDIO_TRACK)) {
                refresh();
            }
            break;
    }
}

void HMI::handleAudioVolume(const AudioVolumeState* event) {
    setVal("audio.mute", event->mute());
    setVolume(event->volume());
    if (!mute_ && !event->mute() && !isPage(HMIPage::SPLASH)) {
        page(HMIPage::AUDIO_VOLUME);
    }
    mute_ = event->mute();
    refresh();
    //TODO: implement fade and balance
}

void HMI::handleAudioTone(const AudioToneState*) {
    //TODO: implement EQ handling
}

void HMI::handleAudioPlayback(const AudioTrackPlaybackState* event) {
    setVal("audio.playback", (uint8_t)event->playback());
    if (event->playback() == AudioPlayback::NO_TRACK)  {
        setTxt("audio_track.telapsed_txt", "");
        setTxt("audio_track.ttotal_txt", "");
        setVal("audio_track.progress_bar", 0);
    } else {
        if (event->time_total() == 0) {
            setTxt("audio_track.telapsed_txt", "");
            setTxt("audio_track.ttotal_txt", "");
        } else {
            setTxtTime("audio_track.telapsed_txt", event->time_elapsed());
            setTxtTime("audio_track.ttotal_txt", event->time_total());
        }
        if (event->time_elapsed() == 0) {
            setVal("audio_track.progress_bar", 0);
        } else if (event->time_total() == 0 || event->time_total() < event->time_elapsed()) {
            setVal("audio_track.progress_bar", 100);
        } else {
            setVal("audio_track.progress_bar",
                    (int32_t)(100 * event->time_elapsed() / event->time_total()));
        }
    }
    if (isPageWithHeader() || isPage(HMIPage::AUDIO_TRACK)) {
        refresh();
    }
}

void HMI::handleAudioSettingsMenu(const AudioSettingsMenuState* event) {
    audio_settings_page_ = event->page();
    audio_settings_count_ = event->count();
    if (!isPage(HMIPage::AUDIO_SETTINGS)) {
        page(HMIPage::AUDIO_SETTINGS);
    } else {
        setVal("audio_settings.navselect", 0);
    }
}

void HMI::setAudioSettingsItem(uint8_t item, uint8_t type) {
    // set item type
    stream_->print("m");
    stream_->print(item);
    stream_->print("_type.val=");
    stream_->print(type);
    terminate();

    // set item label
    stream_->print("m");
    stream_->print(item);
    stream_->print("_txt.txt=\"");
    if (type != 0) {
        printEscaped((char*)scratch_->bytes);
    }
    stream_->print("\"");
    terminate();
}

void HMI::handleAudioSettingsItem(const AudioSettingsItemState* event) {
    if (event->item() > 4) {
        return;
    }

    setAudioSettingsItem(event->item(), (uint8_t)event->type());

    if (event->reload()) {
        refresh();
    } else if (event->item() == audio_settings_count_ - 1) {
        for (uint8_t i = audio_settings_count_; i < 5; ++i) {
            setAudioSettingsItem(i, 0);
        }
        refresh();
    }
}

void HMI::handleAudioSettingsExit(const AudioSettingsExitState*) {
    page(HMIPage::AUDIO);
}

void HMI::handleSerial(const Yield<Message>& yield) {
    if (scratch_->size == 0) {
        return;
    }

    switch (scratch_->bytes[0]) {
        case 0x65:
            // Buttons are triggered on release. 
            if (scratch_->size >= 4 && scratch_->bytes[3] == 0) {
                uint8_t button = scratch_->bytes[2];
                switch ((HMIPage)scratch_->bytes[1]) {
                    case HMIPage::CLIMATE:
                        handleClimateButton(button, yield);
                        break;
                    case HMIPage::AUDIO_RADIO:
                        handleAudioRadioButton(button, yield);
                        break;
                    case HMIPage::AUDIO_TRACK:
                        handleAudioTrackButton(button, yield);
                        break;
                    case HMIPage::AUDIO_SOURCE:
                        handleAudioSourceButton(button, yield);
                        break;
                    case HMIPage::AUDIO_SETTINGS:
                        handleAudioSettingsButton(button, yield);
                        break;
                    case HMIPage::VEHICLE:
                        handleVehicleButton(button, yield);
                        break;
                    case HMIPage::SETTINGS_1:
                        handleSettings1Button(button, yield);
                        break;
                    case HMIPage::SETTINGS_2:
                        handleSettings2Button(button, yield);
                        break;
                    case HMIPage::SETTINGS_3:
                        handleSettings3Button(button, yield);
                        break;
                    default:
                        break;
                }
            }
            break;
        case 0x66:
            if (scratch_->size >= 2) {
                if (page_.page((HMIPage)scratch_->bytes[1])) {
                    yield(page_);
                }
            }
            break;
        case 0x86:
            if (sleep_.sleep(true)) {
                yield(sleep_);
            }
            break;
        case 0x87:
            if (sleep_.sleep(false)) {
                yield(sleep_);
            }
            break;
        default:
            break;
    }
}

void HMI::handleClimateButton(uint8_t button, const Yield<Message>& yield) {
    switch (button) {
        case 0x11:
            if (climate_system_ == CLIMATE_SYSTEM_OFF) {
                sendCmd(yield, ClimateEvent::TOGGLE_AUTO_CMD);
            } else {
                sendCmd(yield, ClimateEvent::TURN_OFF_CMD);
            }
            break;
        case 0x12:
            sendCmd(yield, ClimateEvent::TOGGLE_AC_CMD);
            break;
        case 0x13:
            sendCmd(yield, ClimateEvent::TOGGLE_RECIRCULATE_CMD);
            break;
        case 0x14:
            sendCmd(yield, ClimateEvent::CYCLE_AIRFLOW_MODE_CMD);
            break;
        default:
            break;
    }
}

void HMI::handleAudioRadioButton(uint8_t button, const Yield<Message>& yield) {
    switch (button) {
        case 11:
            sendCmd(yield, AudioEvent::RADIO_TOGGLE_SEEK_CMD);
            break;
        default:
            break;
    }
}

void HMI::handleAudioTrackButton(uint8_t button, const Yield<Message>& yield) {
    switch (button) {
        case 14:
            sendCmd(yield, AudioEvent::SETTINGS_OPEN_CMD);
            break;
    }
}

void HMI::handleAudioSettingsButton(uint8_t button, const Yield<Message>& yield) {
    switch (button) {
        case 30:
            // on page exit
            sendCmd(yield, AudioEvent::SETTINGS_EXIT_CMD);
            break;
        case 14:
            // on back button
            sendCmd(yield, AudioEvent::SETTINGS_BACK_CMD);
            break;
        case 1:
        case 6:
            sendCmd(yield, AudioEvent::SETTINGS_SELECT_CMD, 0x00);
            break;
        case 5:
        case 7:
            sendCmd(yield, AudioEvent::SETTINGS_SELECT_CMD, 0x01);
            break;
        case 4:
        case 8:
            sendCmd(yield, AudioEvent::SETTINGS_SELECT_CMD, 0x02);
            break;
        case 3:
        case 9:
            sendCmd(yield, AudioEvent::SETTINGS_SELECT_CMD, 0x03);
            break;
        case 2:
        case 15:
            sendCmd(yield, AudioEvent::SETTINGS_SELECT_CMD, 0x04);
            break;
    }
}

void HMI::handleAudioSourceButton(uint8_t button, const Yield<Message>& yield) {
    switch (button) {
        case 0x01:
            sendCmd(yield, AudioEvent::SOURCE_SET_CMD, AudioSource::BLUETOOTH);
            break;
        case 0x02:
            sendCmd(yield, AudioEvent::SOURCE_SET_CMD, AudioSource::AM);
            break;
        case 0x03:
            sendCmd(yield, AudioEvent::SOURCE_SET_CMD, AudioSource::FM);
            break;
        case 0x04:
            sendCmd(yield, AudioEvent::SOURCE_SET_CMD, AudioSource::AUX);
            break;
        case 0x0A:
            sendCmd(yield, AudioEvent::SOURCE_SET_CMD, AudioSource::OPTICAL);
            break;
        default:
            break;
    }
}

void HMI::handleVehicleButton(uint8_t button, const Yield<Message>& yield) {
    switch (button) {
        case 0x17:
            {
                uint8_t tires = getVal("vehicle.swap_tires");
                if ((tires & 0x0F) != ((tires >> 4) & 0x0F)) {
                    sendCmd(yield, TireEvent::SWAP_POSITION_CMD, tires);
                }
            }
            break;
        default:
            break;
    }
}

void HMI::handleSettings1Button(uint8_t button, const Yield<Message>& yield) {
    switch (button) {
        case 0x05:
            sendCmd(yield, SettingsEvent::TOGGLE_AUTO_INTERIOR_ILLUM_CMD);
            break;
        case 0x07:
            sendCmd(yield, SettingsEvent::PREV_AUTO_HEADLIGHT_SENS_CMD);
            break;
        case 0x08:
            sendCmd(yield, SettingsEvent::NEXT_AUTO_HEADLIGHT_SENS_CMD);
            break;
        case 0x0B:
            sendCmd(yield, SettingsEvent::PREV_AUTO_HEADLIGHT_OFF_DELAY_CMD);
            break;
        case 0x0C:
            sendCmd(yield, SettingsEvent::NEXT_AUTO_HEADLIGHT_OFF_DELAY_CMD);
            break;
        case 0x0F:
            sendCmd(yield, SettingsEvent::TOGGLE_SPEED_SENSING_WIPER_CMD);
            break;
        case 0x11:
            sendCmd(yield, SettingsEvent::TOGGLE_REMOTE_KEY_RESP_HORN_CMD);
            break;
        default:
            break;
    }
}

void HMI::handleSettings2Button(uint8_t button, const Yield<Message>& yield) {
    switch (button) {
        case 0x0A:
            sendCmd(yield, SettingsEvent::PREV_REMOTE_KEY_RESP_LIGHTS_CMD);
            break;
        case 0x04:
            sendCmd(yield, SettingsEvent::NEXT_REMOTE_KEY_RESP_LIGHTS_CMD);
            break;
        case 0x06:
            sendCmd(yield, SettingsEvent::PREV_AUTO_RELOCK_TIME_CMD);
            break;
        case 0x07:
            sendCmd(yield, SettingsEvent::NEXT_AUTO_RELOCK_TIME_CMD);
            break;
        case 0x0B:
            sendCmd(yield, SettingsEvent::TOGGLE_SELECTIVE_DOOR_UNLOCK_CMD);
            break;
        case 0x0E:
            sendCmd(yield, SettingsEvent::TOGGLE_SLIDE_DRIVER_SEAT_CMD);
            break;
        case 0x10:
            sendCmd(yield, BluetoothEvent::FORGET_CMD);
            break;
        default:
            break;
    }
}

void HMI::handleSettings3Button(uint8_t button, const Yield<Message>& yield) {
    Event event;
    switch (button) {
        case 0x03:
            sendCmd(yield, SettingsEvent::FACTORY_RESET_CMD);
            break;
        default:
            break;
    }
}

void HMI::navUp(const Yield<Message>& yield) {
    uint8_t selected = getVal("navselect");
    switch (page_.page()) {
        case HMIPage::AUDIO_SETTINGS:
        case HMIPage::AUDIO_SOURCE:
        case HMIPage::AUDIO_EQ:
        case HMIPage::SETTINGS_1:
            if (selected > 1) {
                setVal("navselect", selected - 1);
                refresh();
            }
            break;
        case HMIPage::SETTINGS_2:
            if (selected <= 1) {
                setVal("settings_1.navselect", 5);
                page(HMIPage::SETTINGS_1);
            } else {
                setVal("navselect", selected - 1);
                refresh();
            }
            break;
        case HMIPage::SETTINGS_3:
            if (selected <= 1) {
                setVal("settings_2.navselect", 5);
                page(HMIPage::SETTINGS_1);
            }
            break;
        default:
            break;
    }
}

void HMI::navDown(const Yield<Message>& yield) {
    uint8_t selected = getVal("navselect");
    switch (page_.page()) {
        case HMIPage::AUDIO_SETTINGS:
            if (selected < audio_settings_count_) {
                setVal("navselect", selected + 1);
                refresh();
            }
            break;
        case HMIPage::AUDIO_SOURCE:
        case HMIPage::AUDIO_EQ:
            if (selected < 5) {
                setVal("navselect", selected + 1);
                refresh();
            }
            break;
        case HMIPage::SETTINGS_1:
            if (selected >= 5) {
                setVal("settings_2.navselect", 1);
                page(HMIPage::SETTINGS_2);
            } else {
                setVal("navselect", selected + 1);
                refresh();
            }
            break;
        case HMIPage::SETTINGS_2:
            if (selected >= 5) {
                setVal("settings_3.navselect", 1);
                page(HMIPage::SETTINGS_3);
            } else {
                setVal("navselect", selected + 1);
                refresh();
            }
            break;
        case HMIPage::SETTINGS_3:
            if (selected < 1) {
                setVal("navselect", selected + 1);
                refresh();
            }
            break;
        default:
            break;
    }
}

void HMI::navLeft(const Yield<Message>& yield) {
    uint8_t selected = getVal("navselect");
    switch (page_.page()) {
        case HMIPage::AUDIO_EQ:
            if (selected == 1) {
                sendCmd(yield, AudioEvent::TONE_BASS_DEC_CMD);
            } else if (selected == 2) {
                sendCmd(yield, AudioEvent::TONE_MID_DEC_CMD);
            } else if (selected == 3) {
                sendCmd(yield, AudioEvent::TONE_TREBLE_DEC_CMD);
            } else if (selected == 4) {
                sendCmd(yield, AudioEvent::FADE_REAR_CMD);
            } else if (selected == 5) {
                sendCmd(yield, AudioEvent::BALANCE_LEFT_CMD);
            }
            break;
        case HMIPage::SETTINGS_1:
            if (selected == 2) {
                sendCmd(yield, SettingsEvent::PREV_AUTO_HEADLIGHT_SENS_CMD);
            } else if (selected == 3) {
                sendCmd(yield, SettingsEvent::PREV_AUTO_HEADLIGHT_OFF_DELAY_CMD);
            }
            break;
        case HMIPage::SETTINGS_2:
            if (selected == 1) {
                sendCmd(yield, SettingsEvent::PREV_REMOTE_KEY_RESP_LIGHTS_CMD);
            } else if (selected == 2) {
                sendCmd(yield, SettingsEvent::PREV_AUTO_RELOCK_TIME_CMD);
            }
            break;
        default:
            break;
    }
}

void HMI::navRight(const Yield<Message>& yield) {
    uint8_t selected = getVal("navselect");
    switch (page_.page()) {
        case HMIPage::AUDIO_EQ:
            if (selected == 1) {
                sendCmd(yield, AudioEvent::TONE_BASS_INC_CMD);
            } else if (selected == 2) {
                sendCmd(yield, AudioEvent::TONE_MID_INC_CMD);
            } else if (selected == 3) {
                sendCmd(yield, AudioEvent::TONE_TREBLE_INC_CMD);
            } else if (selected == 4) {
                sendCmd(yield, AudioEvent::FADE_FRONT_CMD);
            } else if (selected == 5) {
                sendCmd(yield, AudioEvent::BALANCE_RIGHT_CMD);
            }
            break;
        case HMIPage::SETTINGS_1:
            if (selected == 2) {
                sendCmd(yield, SettingsEvent::NEXT_AUTO_HEADLIGHT_SENS_CMD);
            } else if (selected == 3) {
                sendCmd(yield, SettingsEvent::NEXT_AUTO_HEADLIGHT_OFF_DELAY_CMD);
            }
            break;
        case HMIPage::SETTINGS_2:
            if (selected == 1) {
                sendCmd(yield, SettingsEvent::NEXT_REMOTE_KEY_RESP_LIGHTS_CMD);
            } else if (selected == 2) {
                sendCmd(yield, SettingsEvent::NEXT_AUTO_RELOCK_TIME_CMD);
            }
            break;
        default:
            break;
    }
}

void HMI::navActivate(const Yield<Message>& yield) {
    uint8_t selected = getVal("navselect");
    switch (page_.page()) {
        case HMIPage::AUDIO_SETTINGS:
            if (selected <= audio_settings_count_) {
                sendCmd(yield, AudioEvent::SETTINGS_SELECT_CMD, selected - 1);
            }
            break;
        case HMIPage::AUDIO_SOURCE:
            if (selected == 1) {
                sendCmd(yield, AudioEvent::SOURCE_SET_CMD, AudioSource::BLUETOOTH);
            } else if (selected == 2) {
                sendCmd(yield, AudioEvent::SOURCE_SET_CMD, AudioSource::AM);
            } else if (selected == 3) {
                sendCmd(yield, AudioEvent::SOURCE_SET_CMD, AudioSource::FM);
            } else if (selected == 4) {
                sendCmd(yield, AudioEvent::SOURCE_SET_CMD, AudioSource::AUX);
            } else if (selected == 5) {
                sendCmd(yield, AudioEvent::SOURCE_SET_CMD, AudioSource::OPTICAL);
            }
            break;
        case HMIPage::SETTINGS_1:
            if (selected == 1) {
                sendCmd(yield, SettingsEvent::TOGGLE_AUTO_INTERIOR_ILLUM_CMD);
            } else if (selected == 4) {
                sendCmd(yield, SettingsEvent::TOGGLE_SPEED_SENSING_WIPER_CMD);
            } else if (selected == 5) {
                sendCmd(yield, SettingsEvent::TOGGLE_REMOTE_KEY_RESP_HORN_CMD);
            }
            break;
        case HMIPage::SETTINGS_2:
            if (selected == 3) {
                sendCmd(yield, SettingsEvent::TOGGLE_SELECTIVE_DOOR_UNLOCK_CMD);
            } else if (selected == 4) {
                sendCmd(yield, SettingsEvent::TOGGLE_SLIDE_DRIVER_SEAT_CMD);
            } else if (selected == 5) {
                sendCmd(yield, BluetoothEvent::FORGET_CMD);
            }
            break;
        case HMIPage::SETTINGS_3:
            if (selected == 1) {
                sendCmd(yield, SettingsEvent::FACTORY_RESET_CMD);
            }
            break;
        default:
            break;
    }
}

void HMI::emit(const Yield<Message>& yield) {
    if (read(false)) {
        Serial.print("hmi recv: ");
        for (size_t i = 0; i < scratch_->size; ++i) {
            if (scratch_->bytes[i] < 0x0F) {
                Serial.print("0");
            }
            Serial.print(scratch_->bytes[i], HEX);
        }
        Serial.println();
        handleSerial(yield);
    }
}

void HMI::terminate() {
    stream_->write(0xFF);
    stream_->write(0xFF);
    stream_->write(0xFF);
}

void HMI::refresh() {
    stream_->print("click refresh,1");
    terminate();
}

void HMI::show(const char* obj) {
    stream_->print("vis ");
    stream_->print(obj);
    stream_->print(",1");
    terminate();
}

void HMI::hide(const char* obj) {
    stream_->print("vis ");
    stream_->print(obj);
    stream_->print(",0");
    terminate();
}

bool HMI::isPage(HMIPage value) {
    return page_.page() == value;
}

bool HMI::isAudioPage() {
    return isPage(HMIPage::AUDIO) ||
        isPage(HMIPage::AUDIO_TRACK) ||
        isPage(HMIPage::AUDIO_RADIO) ||
        isPage(HMIPage::AUDIO_AUX) ||
        isPage(HMIPage::AUDIO_POWER_OFF) ||
        isPage(HMIPage::AUDIO_NO_STEREO) ||
        isPage(HMIPage::AUDIO_VOLUME) ||
        isPage(HMIPage::AUDIO_SOURCE) ||
        isPage(HMIPage::AUDIO_SETTINGS);
}

bool HMI::isAudioSourcePage() {
    return isPage(HMIPage::AUDIO_TRACK) ||
        isPage(HMIPage::AUDIO_RADIO) ||
        isPage(HMIPage::AUDIO_AUX) ||
        isPage(HMIPage::AUDIO_POWER_OFF) ||
        isPage(HMIPage::AUDIO_NO_STEREO) ||
        isPage(HMIPage::AUDIO_VOLUME) ||
        isPage(HMIPage::AUDIO_SOURCE);
}

bool HMI::isSettingsPage() {
    return isPage(HMIPage::AUDIO_SOURCE) ||
        isPage(HMIPage::AUDIO_SETTINGS) ||
        isPage(HMIPage::AUDIO_EQ) ||
        isPage(HMIPage::SETTINGS) ||
        isPage(HMIPage::SETTINGS_1) ||
        isPage(HMIPage::SETTINGS_2) ||
        isPage(HMIPage::SETTINGS_3);
}

bool HMI::isPageWithHeader() {
    return isPage(HMIPage::HOME) ||
        isPage(HMIPage::CLIMATE) ||
        isPage(HMIPage::AUDIO_TRACK) ||
        isPage(HMIPage::AUDIO_RADIO) ||
        isPage(HMIPage::AUDIO_AUX) ||
        isPage(HMIPage::AUDIO_POWER_OFF) ||
        isPage(HMIPage::AUDIO_NO_STEREO) ||
        isPage(HMIPage::AUDIO_VOLUME) ||
        isPage(HMIPage::VEHICLE);
}

void HMI::page(HMIPage value) {
    stream_->print("page ");
    stream_->print((int32_t)value);
    terminate();
}

void HMI::climatePopup() {
    if (!isSettingsPage() && !isPage(HMIPage::CLIMATE)) {
        setVal("climate.popup", 1);
        page(HMIPage::CLIMATE);
    }
}

void HMI::printEscaped(const char* value) {
    for (size_t i = 0; value[i] != 0; ++i) {
        if (value[i] == '"' || value[i] == '\\') {
            stream_->print('\\');
        }
        stream_->print(value[i]);
    }
}

int32_t HMI::getVal(const char* key) {
    stream_->print("get ");
    stream_->print(key);
    stream_->print(".val");
    terminate();
    if (!read(true) || scratch_->size < 5 || scratch_->bytes[0] != 0x71) {
        return 0;
    }
    union {
       int32_t sl;
       uint32_t ul;
    } value;
    value.ul = Endian::btohl(scratch_->bytes, Endian::BIG);
    return value.sl;
}

void HMI::setVal(const char* key, int32_t value) {
    stream_->print(key);
    stream_->print(".val=");
    stream_->print(value);
    terminate();
}

void HMI::setTxt(const char* key, int32_t value) {
    stream_->print(key);
    stream_->print(".txt=\"");
    stream_->print(value);
    stream_->print("\"");
    terminate();
}

void HMI::setTxt(const char* key, double value, uint8_t precision) {
    stream_->print(key);
    stream_->print(".txt=\"");
    stream_->print(value, precision);
    stream_->print("\"");
    terminate();
}

void HMI::setTxt(const char* key, const char* value) {
    stream_->print(key);
    stream_->print(".txt=\"");
    printEscaped(value);
    stream_->print("\"");
    terminate();
}

void HMI::setTxt(const char* key, Scratch* scratch) {
    stream_->print(key);
    stream_->print(".txt=\"");
    printEscaped((char*)scratch->bytes);
    stream_->print("\"");
    terminate();
}

void HMI::setTxtTemp(const char* key, int32_t degrees) {
    stream_->print(key);
    stream_->print(".txt=\"");
    stream_->print(degrees);
    stream_->print((char)0xB0);
    stream_->print("\"");
    terminate();
}

void HMI::setTxtTime(const char* key, uint16_t seconds) {
    stream_->print(key);
    stream_->print(".txt=\"");
    stream_->print((int16_t)(seconds / 60));
    stream_->print(":");
    uint16_t s = seconds % 60;
    if (s < 10) {
        stream_->print("0");
    }
    stream_->print(s);
    stream_->print("\"");
    terminate();
}

void HMI::setVolume(uint8_t value) {
    stream_->print("audio_volume.volume_txt.txt=\"");
    if (value < 10) {
        stream_->print("0");
    }
    stream_->print(value);
    stream_->print("\"");
    terminate();
    setVal("audio_volume.volume_bar", (int32_t)(100 * value / 24));
}

void HMI::setGain(int8_t db) {
    stream_->print("audio_aux.gain_txt.txt=\"");
    stream_->print(db);
    stream_->print(" dB\"");
    terminate();
}

bool HMI::read(bool block) {
    if (!block && !stream_->available()) {
        return false;
    }

    int b;
    int n = 0;
    size_t terminate = 0;

    while (terminate < 3) {
        b = stream_->read();
        if (b == -1) {
            continue;
        }

        if ((size_t)n >= kScratchCapacity) {
            n = -1;
        } else if (n >= 0) {
            scratch_->bytes[n++] = (uint8_t)b;
        }
        if (b == 0xFF) {
            ++terminate;
        } else {
            terminate = 0;
        }
    }
    if (n == -1) {
        return 0;
    }
    scratch_->size = n - 3;
    return n - 3;
}

}  // namespace R51
