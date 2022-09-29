#include "HMI.h"

#include <Arduino.h>
#include <Bluetooth.h>
#include <Caster.h>
#include <Common.h>
#include <Vehicle.h>
#include "Fusion.h"

namespace R51 {

HMI::HMI(Stream* stream, Scratch* scratch) :
        stream_(new HMIDebugStream(stream)), scratch_(scratch), climate_system_(0) {}

void HMI::handle(const Message& msg, const Caster::Yield<Message>&) {
    if (msg.type() != Message::EVENT) {
        return;
    }
    const auto& event = msg.event();
    switch ((SubSystem)event.subsystem) {
        case SubSystem::ECM:
            Serial.println("ecm event");
            handleECM(event);
            break;
        case SubSystem::IPDM:
            Serial.println("ipdm event");
            handleIPDM(event);
            break;
        case SubSystem::TIRE:
            Serial.println("tire event");
            handleTire(event);
            break;
        case SubSystem::CLIMATE:
            switch ((ClimateEvent)event.id) {
                case ClimateEvent::SYSTEM_STATE:
                    handleClimateSystem((ClimateSystemStateEvent*)&event);;
                    break;
                case ClimateEvent::AIRFLOW_STATE:
                    handleClimateAirflow((ClimateAirflowStateEvent*)&event);
                    break;
                case ClimateEvent::TEMP_STATE:
                    handleClimateTemp((ClimateTempStateEvent*)&event);
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

void HMI::handleClimateSystem(const ClimateSystemStateEvent* event) {
    climate_system_ = (uint8_t)event->mode();
    setVal("climate.system", climate_system_);
    setVal("climate.ac", event->ac());
    setVal("climate.dual", event->dual());
    if (isPage(HMIPage::CLIMATE)) {
        refresh();
    }
}

void HMI::handleClimateAirflow(const ClimateAirflowStateEvent* event) {
    setVal("climate.fan_bar", (int32_t)(100 * event->fan_speed() / 7));
    setVal("climate.vface", event->face());
    setVal("climate.vfeet", event->feet());
    setVal("climate.vdefog", event->windshield());
    setVal("climate.recirc", event->recirculate());
    if (isPage(HMIPage::CLIMATE)) {
        refresh();
    }
}

void HMI::handleClimateTemp(const ClimateTempStateEvent* event) {
    setTxtTemp("climate.dtemp_txt", event->driver_temp());
    setTxtTemp("climate.ptemp_txt", event->passenger_temp());
    setTxtTemp("climate.otemp_txt", event->outside_temp());
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
            setVal("audio_track.device", event->bt_connected());
            if (isPage(HMIPage::AUDIO_TRACK)) {
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

void HMI::handleSerial(const Caster::Yield<Message>& yield) {
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
                    case HMIPage::AUDIO_SOURCE:
                        handleAudioSourceButton(button, yield);
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
                        Serial.print(scratch_->bytes[1]);
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

void HMI::handleClimateButton(uint8_t button, const Caster::Yield<Message>& yield) {
    Event event;
    switch (button) {
        case 0x11:
            if (climate_system_ == 0x00) {
                event = Event(SubSystem::CLIMATE,
                            (uint8_t)ClimateEvent::TOGGLE_AUTO);
            } else {
                event = Event(SubSystem::CLIMATE,
                            (uint8_t)ClimateEvent::TURN_OFF);
            }
            yield(event);
            break;
        case 0x12:
            event = Event(SubSystem::CLIMATE,
                        (uint8_t)ClimateEvent::TOGGLE_AC);
            yield(event);
            break;
        case 0x13:
            event = Event(SubSystem::CLIMATE,
                        (uint8_t)ClimateEvent::TOGGLE_RECIRCULATE);
            yield(event);
            break;
        case 0x14:
            event = Event(SubSystem::CLIMATE,
                        (uint8_t)ClimateEvent::CYCLE_AIRFLOW_MODE);
            yield(event);
            break;
        default:
            Serial.println(button, HEX);
            break;
    }
}

void HMI::handleAudioRadioButton(uint8_t button, const Caster::Yield<Message>& yield) {
    switch (button) {
        case 0x0B:
            break;
        default:
            Serial.println(button, HEX);
            break;
    }
}

void HMI::handleAudioSourceButton(uint8_t button, const Caster::Yield<Message>& yield) {
    Event event;
    switch (button) {
        case 0x01:
            event = Event(SubSystem::AUDIO,
                        (uint8_t)AudioEvent::SOURCE_SET,
                        {(uint8_t)AudioSource::BLUETOOTH});
            yield(event);
            break;
        case 0x02:
            event = Event(SubSystem::AUDIO,
                        (uint8_t)AudioEvent::SOURCE_SET,
                        {(uint8_t)AudioSource::AM});
            yield(event);
            break;
        case 0x03:
            event = Event(SubSystem::AUDIO,
                        (uint8_t)AudioEvent::SOURCE_SET,
                        {(uint8_t)AudioSource::FM});
            yield(event);
            break;
        case 0x04:
            event = Event(SubSystem::AUDIO,
                        (uint8_t)AudioEvent::SOURCE_SET,
                        {(uint8_t)AudioSource::AUX});
            yield(event);
            break;
        case 0x0A:
            event = Event(SubSystem::AUDIO,
                        (uint8_t)AudioEvent::SOURCE_SET,
                        {(uint8_t)AudioSource::OPTICAL});
            yield(event);
            break;
        default:
            Serial.println(button, HEX);
            break;
    }
}

void HMI::handleVehicleButton(uint8_t button, const Caster::Yield<Message>& yield) {
    switch (button) {
        case 0x17:
            {
                uint8_t tires = get("vehicle.swap_tires");
                if ((tires & 0x0F) != ((tires >> 4) & 0x0F)) {
                    Event event = Event(SubSystem::TIRE,
                                (uint8_t)TireEvent::SWAP_POSITION,
                                {tires});
                    yield(event);
                }
            }
            break;
        default:
            Serial.println(button, HEX);
            break;
    }
}

void HMI::handleSettings1Button(uint8_t button, const Caster::Yield<Message>& yield) {
    Event event;
    switch (button) {
        case 0x05:
            event = Event(SubSystem::SETTINGS,
                        (uint8_t)SettingsEvent::TOGGLE_AUTO_INTERIOR_ILLUMINATAION);
            yield(event);
            break;
        case 0x07:
            event = Event(SubSystem::SETTINGS,
                        (uint8_t)SettingsEvent::PREV_AUTO_HEADLIGHT_SENSITIVITY);
            yield(event);
            break;
        case 0x08:
            event = Event(SubSystem::SETTINGS,
                        (uint8_t)SettingsEvent::NEXT_AUTO_HEADLIGHT_SENSITIVITY);
            yield(event);
            break;
        case 0x0B:
            event = Event(SubSystem::SETTINGS,
                        (uint8_t)SettingsEvent::PREV_AUTO_HEADLIGHT_OFF_DELAY);
            yield(event);
            break;
        case 0x0C:
            event = Event(SubSystem::SETTINGS,
                        (uint8_t)SettingsEvent::NEXT_AUTO_HEADLIGHT_OFF_DELAY);
            yield(event);
            break;
        case 0x0F:
            event = Event(SubSystem::SETTINGS,
                        (uint8_t)SettingsEvent::TOGGLE_SPEED_SENSING_WIPER_INTERVAL);
            yield(event);
            break;
        case 0x11:
            event = Event(SubSystem::SETTINGS,
                        (uint8_t)SettingsEvent::TOGGLE_REMOTE_KEY_RESPONSE_HORN);
            yield(event);
            break;
        default:
            Serial.println(button, HEX);
            break;
    }
}

void HMI::handleSettings2Button(uint8_t button, const Caster::Yield<Message>& yield) {
    Event event;
    switch (button) {
        case 0x0A:
            event = Event(SubSystem::SETTINGS,
                        (uint8_t)SettingsEvent::PREV_REMOTE_KEY_RESPONSE_LIGHTS);
            yield(event);
            break;
        case 0x04:
            event = Event(SubSystem::SETTINGS,
                        (uint8_t)SettingsEvent::NEXT_REMOTE_KEY_RESPONSE_LIGHTS);
            yield(event);
            break;
        case 0x06:
            event = Event(SubSystem::SETTINGS,
                        (uint8_t)SettingsEvent::PREV_AUTO_RELOCK_TIME);
            yield(event);
            break;
        case 0x07:
            event = Event(SubSystem::SETTINGS,
                        (uint8_t)SettingsEvent::NEXT_AUTO_RELOCK_TIME);
            yield(event);
            break;
        case 0x0B:
            event = Event(SubSystem::SETTINGS,
                        (uint8_t)SettingsEvent::TOGGLE_SELECTIVE_DOOR_UNLOCK);
            yield(event);
            break;
        case 0x0E:
            event = Event(SubSystem::SETTINGS,
                        (uint8_t)SettingsEvent::TOGGLE_SLIDE_DRIVER_SEAT_BACK_ON_EXIT);
            yield(event);
            break;
        case 0x10:
            event = Event(SubSystem::BLUETOOTH,
                        (uint8_t)BluetoothEvent::FORGET);
            yield(event);
            break;
        default:
            Serial.println(button, HEX);
            break;
    }
}

void HMI::handleSettings3Button(uint8_t button, const Caster::Yield<Message>& yield) {
    Event event;
    switch (button) {
        case 0x03:
            event = Event(SubSystem::SETTINGS,
                        (uint8_t)SettingsEvent::FACTORY_RESET);
            yield(event);
            break;
        default:
            Serial.println(button, HEX);
            break;
    }
}

void HMI::emit(const Caster::Yield<Message>& yield) {
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

void HMI::printEscaped(const char* value) {
    for (size_t i = 0; value[i] != 0; ++i) {
        if (value[i] == '"' || value[i] == '\\') {
            stream_->print('\\');
        }
        stream_->print(value[i]);
    }
}

int32_t HMI::get(const char* key) {
    stream_->print("get ");
    stream_->print(key);
    stream_->print(".val");
    terminate();
    if (!read(true) || scratch_->size < 5 || scratch_->bytes[0] != 0x71) {
        return 0;
    }
    int32_t value;
    memcpy(&value, scratch_->bytes + 1, 4);
    return value;
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

}
