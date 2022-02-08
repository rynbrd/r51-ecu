#include "vehicle.h"

#include <Arduino.h>
#include "binary.h"
#include "debug.h"


// Send buffer for SettingsCommand.
byte buffer_[8];

inline bool matchFrame(byte* data, byte prefix0) {
    return data[0] == prefix0;
}

inline bool matchFrame(byte* data, byte prefix0, byte prefix1, byte prefix2) {
    return data[0] == prefix0 && data[1] == prefix1 && data[2] == prefix2;
}

SettingsCommand::Op SettingsCommand::op() const {
    return op_;
}

bool SettingsCommand::ready() {
    return op_ == OP_READY;
}

void SettingsCommand::loop() {
    if (millis() - last_write_ > 5000) {
        if (op_ == OP_READY) {
            return;
        }
        ERROR_MSG("vehicle: settings command timed out");
        if (op_ != OP_EXIT) {
            sendRequest(OP_EXIT);
        }
        op_ = OP_READY;
    }
}

bool SettingsCommand::send() {
    if (id_ == FRAME_UNSET || op_ != OP_READY) {
        return false;
    }
    return sendRequest(OP_ENTER);
}

bool SettingsCommand::sendControl(Op op, byte prefix0, byte prefix1, byte prefix2, byte value) {
    if (id_ == 0) {
        return false;
    }
    buffer_[0] = prefix0;
    buffer_[1] = prefix1;
    buffer_[2] = prefix2;
    buffer_[3] = value;
    memset(buffer_+4, 0xFF, 4);
    if (!conn_->write(id_, 8, buffer_)) {
        return false;
    }
    op_ = op;
    last_write_ = millis();
    INFO_MSG_FRAME("vehicle: write frame ", id_, 8, buffer_);
    return true;
}

bool SettingsCommand::sendRequest(Op op, uint8_t value) {
    switch (op) {
        default: 
        case OP_READY:
            op_ = OP_READY;
            return true;
        case OP_ENTER:
            return sendControl(op, 0x02, 0x10, 0xC0);
        case OP_EXIT:
            return sendControl(op, 0x02, 0x10, 0x81);
        case OP_INIT_00:
            return sendControl(op, 0x02, 0x3B, 0x00);
        case OP_INIT_20:
            return sendControl(op, 0x02, 0x3B, 0x20);
        case OP_INIT_40:
            return sendControl(op, 0x02, 0x3B, 0x40);
        case OP_INIT_60:
            return sendControl(op, 0x02, 0x3B, 0x60);
        case OP_AUTO_INTERIOR_ILLUM:
            return sendControl(op, 0x03, 0x3B, 0x10, value);
        case OP_AUTO_HL_SENS:
            return sendControl(op, 0x03, 0x3B, 0x37, value);
        case OP_AUTO_HL_DELAY:
            return sendControl(op, 0x03, 0x3B, 0x39, value);
        case OP_SPEED_SENS_WIPER:
            return sendControl(op, 0x03, 0x3B, 0x47, value);
        case OP_REMOTE_KEY_HORN:
            return sendControl(op, 0x03, 0x3B, 0x2A, value);
        case OP_REMOTE_KEY_LIGHT:
            return sendControl(op, 0x03, 0x3B, 0x2E, value);
        case OP_AUTO_RELOCK_TIME:
            return sendControl(op, 0x03, 0x3B, 0x2F, value);
        case OP_SELECT_DOOR_UNLOCK:
            return sendControl(op, 0x03, 0x3B, 0x02, value);
        case OP_SLIDE_DRIVER_SEAT:
            return sendControl(op, 0x03, 0x3B, 0x01, value);
        case OP_GET_STATE_71E_10:
            return sendControl(op, 0x02, 0x21, 0x01);
        case OP_GET_STATE_71E_2X:
            return sendControl(op, 0x30, 0x00, 0x0A);
    }
}

bool SettingsCommand::matchResponse(uint32_t id, uint8_t len, byte* data) {
    if (id != ((id_ & ~0x010) | 0x020) || len != 8) {
        return false;
    }
    switch (op_) {
        default:
        case OP_READY:
            return true;
        case OP_ENTER:
            return matchFrame(data, 0x02, 0x50, 0xC0);
        case OP_EXIT:
            return matchFrame(data, 0x02, 0x50, 0x81);
        case OP_INIT_00:
            return matchFrame(data, 0x06, 0x7B, 0x00);
        case OP_INIT_20:
            return matchFrame(data, 0x06, 0x7B, 0x20);
        case OP_INIT_40:
            return matchFrame(data, 0x06, 0x7B, 0x40);
        case OP_INIT_60:
            return matchFrame(data, 0x06, 0x7B, 0x60);
        case OP_AUTO_HL_SENS:
            return matchFrame(data, 0x02, 0x7B, 0x37);
        case OP_AUTO_HL_DELAY:
            return matchFrame(data, 0x02, 0x7B, 0x39);
        case OP_SPEED_SENS_WIPER:
            return matchFrame(data, 0x02, 0x7B, 0x47);
        case OP_REMOTE_KEY_HORN:
            return matchFrame(data, 0x02, 0x7B, 0x2A);
        case OP_REMOTE_KEY_LIGHT:
            return matchFrame(data, 0x02, 0x7B, 0x2E);
        case OP_AUTO_RELOCK_TIME:
            return matchFrame(data, 0x02, 0x7B, 0x2F);
        case OP_SELECT_DOOR_UNLOCK:
            return matchFrame(data, 0x02, 0x7B, 0x02);
        case OP_SLIDE_DRIVER_SEAT:
            return matchFrame(data, 0x02, 0x7B, 0x01);
        case OP_GET_STATE_71E_10:
            return matchFrame(data, 0x10);
        case OP_GET_STATE_71E_2X:
            return matchFrame(data, 0x21) || matchFrame(data, 0x22);
        case OP_GET_STATE_71F_05:
            return matchFrame(data, 0x05);
    }
}

bool SettingsCommand::matchAndSend(uint32_t id, uint8_t len, byte* data, Op match, Op send, uint8_t value) {
    if (op_ != match || !matchResponse(id, len, data)) {
        return false;
    }
    return sendRequest(send, value);
}

void SettingsInit::receive(uint32_t id, uint8_t len, byte* data) {
    switch (id_) {
        case FRAME_E:
            matchAndSend(id, len, data, OP_ENTER, OP_INIT_00) ||
            matchAndSend(id, len, data, OP_INIT_00, OP_INIT_20) ||
            matchAndSend(id, len, data, OP_INIT_20, OP_INIT_40) ||
            matchAndSend(id, len, data, OP_INIT_40, OP_INIT_60) ||
            matchAndSend(id, len, data, OP_INIT_60, OP_EXIT) ||
            matchAndSend(id, len, data, OP_EXIT, OP_READY);
            break;
        case FRAME_F:
            matchAndSend(id, len, data, OP_ENTER, OP_INIT_00) ||
            matchAndSend(id, len, data, OP_INIT_00, OP_EXIT) ||
            matchAndSend(id, len, data, OP_EXIT, OP_READY);
            break;
        default:
            matchAndSend(id, len, data, OP_ENTER, OP_EXIT) ||
            matchAndSend(id, len, data, OP_EXIT, OP_READY);
    }
}

void SettingsState::receive(uint32_t id, uint8_t len, byte* data) {
    switch (id_) {
        case FRAME_E:
            if (matchAndSend(id, len, data, OP_ENTER, OP_GET_STATE_71E_10) ||
                matchAndSend(id, len, data, OP_GET_STATE_71E_10, OP_GET_STATE_71E_2X)) {
                return;
            }

            if (op() == OP_GET_STATE_71E_2X && matchResponse(id, len, data)) {
                state_count_++;
                if (state_count_ >= 2) {
                    sendRequest(OP_EXIT);
                    return;
                }
            }
            break;
        case FRAME_F:
            if (matchAndSend(id, len, data, OP_ENTER, OP_GET_STATE_71F_05) ||
                matchAndSend(id, len, data, OP_GET_STATE_71F_05, OP_EXIT)) {
                return;
            }
            break;
        default:
            if (matchAndSend(id, len, data, OP_ENTER, OP_EXIT)) {
                return;
            }
            break;
    }
    matchAndSend(id, len, data, OP_EXIT, OP_READY);
}

bool SettingsUpdate::send(Op setting, uint8_t value) {
    switch (setting) {
        default:
            id_ = FRAME_E;
            break;
        case OP_SLIDE_DRIVER_SEAT:
            id_ = FRAME_F;
            break;
    }
    setting_ = setting;
    value_ = value;
    state_count_ = 0;
    return SettingsCommand::send();
}

void SettingsUpdate::receive(uint32_t id, uint8_t len, byte* data) {
    if (op() == OP_READY || matchAndSend(id, len, data, OP_ENTER, setting_, value_)) {
        return;
    }

    switch (id_) {
        case FRAME_E:
            if (matchAndSend(id, len, data, setting_, OP_GET_STATE_71E_10) ||
                matchAndSend(id, len, data, OP_GET_STATE_71E_10, OP_GET_STATE_71E_2X)) {
                return;
            }

            if (op() == OP_GET_STATE_71E_2X && matchResponse(id, len, data)) {
                state_count_++;
                if (state_count_ >= 2) {
                    sendRequest(OP_EXIT);
                    return;
                }
            }
            break;
        case FRAME_F:
            if (matchAndSend(id, len, data, setting_, OP_GET_STATE_71F_05) ||
                matchAndSend(id, len, data, OP_GET_STATE_71F_05, OP_EXIT)) {
                return;
            }
            break;
        default:
            if (matchAndSend(id, len, data, setting_, OP_EXIT)) {
                return;
            }
            break;
    }
    matchAndSend(id, len, data, OP_EXIT, OP_READY);
}

VehicleSettings::~VehicleSettings() {
    if (initE_ != nullptr) {
        delete initE_;
    }
    if (initF_ != nullptr) {
        delete initF_;
    }
}

void VehicleSettings::connect(Connection* can, DashSettingsController* dash) {
    can_ = can;
    dash_ = dash;
    if (initE_ != nullptr) {
        delete initE_;
    }
    initE_ = new SettingsInit(can, SettingsCommand::FRAME_E);

    if (initF_ != nullptr) {
        delete initF_;
    }
    initF_ = new SettingsInit(can, SettingsCommand::FRAME_F);

    if (stateE_ != nullptr) {
        delete stateE_;
    }
    stateE_ = new SettingsState(can, SettingsCommand::FRAME_E);

    if (stateF_ != nullptr) {
        delete stateF_;
    }
    stateF_ = new SettingsState(can, SettingsCommand::FRAME_F);

    if (setter_ != nullptr) {
        delete setter_;
    }
    setter_ = new SettingsUpdate(can_);

    initE_->send();
    initF_->send();
}

bool VehicleSettings::toggleAutoInteriorIllumination() {
    if (!init_) {
        return false;
    }
    return setter_->send(SettingsCommand::OP_AUTO_INTERIOR_ILLUM, !auto_interior_illum_);
}

bool VehicleSettings::nextAutoHeadlightSensitivity() {
    if (!init_) {
        return false;
    }

    switch (hl_sens_) {
        default:
        case HL_SENS_1:
            return setter_->send(SettingsCommand::OP_AUTO_HL_SENS, HL_SENS_2);
        case HL_SENS_2:
            return setter_->send(SettingsCommand::OP_AUTO_HL_SENS, HL_SENS_3);
        case HL_SENS_3:
            return setter_->send(SettingsCommand::OP_AUTO_HL_SENS, HL_SENS_4);
    }
}

bool VehicleSettings::prevAutoHeadlightSensitivity() {
    if (!init_) {
        return false;
    }

    switch (hl_sens_) {
        case HL_SENS_1:
            return false;
        case HL_SENS_2:
            return setter_->send(SettingsCommand::OP_AUTO_HL_SENS, HL_SENS_1);
        default:
        case HL_SENS_3:
            return setter_->send(SettingsCommand::OP_AUTO_HL_SENS, HL_SENS_2);
        case HL_SENS_4:
            return setter_->send(SettingsCommand::OP_AUTO_HL_SENS, HL_SENS_3);
    }
}

bool VehicleSettings::nextAutoHeadlightOffDelay() {
    if (!init_) {
        return false;
    }

    switch (hl_delay_) {
        default:
        case HL_DELAY_30S:
            return setter_->send(SettingsCommand::OP_AUTO_HL_DELAY, HL_DELAY_45S);
        case HL_DELAY_45S:
            return setter_->send(SettingsCommand::OP_AUTO_HL_DELAY, HL_DELAY_60S);
        case HL_DELAY_0S:
        case HL_DELAY_60S:
        case HL_DELAY_90S:
        case HL_DELAY_120S:
        case HL_DELAY_150S:
            return setter_->send(SettingsCommand::OP_AUTO_HL_DELAY, hl_delay_ + 1);
        case HL_DELAY_180S:
            return false;
    }
}

bool VehicleSettings::prevAutoHeadlightOffDelay() {
    if (!init_) {
        return false;
    }

    switch (hl_delay_) {
        case HL_DELAY_0S:
            return false;
        case HL_DELAY_45S:
            return setter_->send(SettingsCommand::OP_AUTO_HL_DELAY, HL_DELAY_30S);
        default:
        case HL_DELAY_60S:
            return setter_->send(SettingsCommand::OP_AUTO_HL_DELAY, HL_DELAY_45S);
        case HL_DELAY_30S:
        case HL_DELAY_90S:
        case HL_DELAY_120S:
        case HL_DELAY_150S:
        case HL_DELAY_180S:
            return setter_->send(SettingsCommand::OP_AUTO_HL_DELAY, hl_delay_ - 1);
    }
}

bool VehicleSettings::toggleSpeedSensingWiperInterval() {
    if (!init_) {
        return false;
    }
    return setter_->send(SettingsCommand::OP_SPEED_SENS_WIPER, !speed_wiper_);
}

bool VehicleSettings::toggleRemoteKeyResponseHorn() {
    if (!init_) {
        return false;
    }
    return setter_->send(SettingsCommand::OP_REMOTE_KEY_HORN, !remote_horn_);
}

bool VehicleSettings::nextRemoteKeyResponseLights() {
    if (!init_) {
        return false;
    }
    switch (remote_lights_) {
        default:
            return setter_->send(SettingsCommand::OP_REMOTE_KEY_LIGHT, LIGHTS_OFF);
        case LIGHTS_OFF:
        case LIGHTS_UNLOCK:
        case LIGHTS_LOCK:
            return setter_->send(SettingsCommand::OP_REMOTE_KEY_LIGHT, remote_lights_ + 1);
        case LIGHTS_ON:
            return false;
    }
}

bool VehicleSettings::prevRemoteKeyResponseLights() {
    if (!init_) {
        return false;
    }
    switch (remote_lights_) {
        default:
            return setter_->send(SettingsCommand::OP_REMOTE_KEY_LIGHT, LIGHTS_OFF);
        case LIGHTS_OFF:
            return false;
        case LIGHTS_UNLOCK:
        case LIGHTS_LOCK:
        case LIGHTS_ON:
            return setter_->send(SettingsCommand::OP_REMOTE_KEY_LIGHT, remote_lights_ - 1);
    }
}

bool VehicleSettings::nextAutoReLockTime() {
    if (!init_) {
        return false;
    }
    switch (relock_time_) {
        default:
            return setter_->send(SettingsCommand::OP_AUTO_RELOCK_TIME, RELOCK_1M);
        case RELOCK_1M:
        case RELOCK_OFF:
            return setter_->send(SettingsCommand::OP_AUTO_RELOCK_TIME, relock_time_ + 1);
        case RELOCK_5M:
            return false;
    }
}

bool VehicleSettings::prevAutoReLockTime() {
    if (!init_) {
        return false;
    }
    switch (relock_time_) {
        default:
            return setter_->send(SettingsCommand::OP_AUTO_RELOCK_TIME, RELOCK_1M);
        case RELOCK_1M:
            return false;
        case RELOCK_OFF:
        case RELOCK_5M:
            return setter_->send(SettingsCommand::OP_AUTO_RELOCK_TIME, relock_time_ - 1);
    }
}

bool VehicleSettings::toggleSelectiveDoorUnlock() {
    if (!init_) {
        return false;
    }
    return setter_->send(SettingsCommand::OP_SELECT_DOOR_UNLOCK, !selective_unlock_);
}

bool VehicleSettings::toggleSlideDriverSeatBackOnExit() {
    if (!init_) {
        return false;
    }
    return setter_->send(SettingsCommand::OP_SLIDE_DRIVER_SEAT, !slide_seat_);
}

bool VehicleSettings::retrieveSettings() {
    return stateE_->send() && stateF_->send();
}

bool VehicleSettings::resetSettingsToDefault() {
    return false;
}

void VehicleSettings::receive(uint32_t id, uint8_t len, byte* data) {
    if ((id != 0x72E && id != 0x72F) || len != 8) {
        return;
    }
    initE_->receive(id, len, data);
    initF_->receive(id, len, data);
    stateE_->receive(id, len, data);
    stateF_->receive(id, len, data);

    if (id == 0x72F && matchFrame(data, 0x05)) {
        receiveState05(data);
    } else if (id == 0x72E && matchFrame(data, 0x10)) {
        receiveState10(data);
    } else if (id == 0x72E && matchFrame(data, 0x21)) {
        receiveState21(data);
    } else if (id == 0x72E && matchFrame(data, 0x22)) {
        receiveState22(data);
    }
}

void VehicleSettings::receiveState05(byte* data) {
    slide_seat_ = getBit(data, 3, 0);
    dash_->setSlideDriverSeatBackOnExit(slide_seat_);
}

void VehicleSettings::receiveState10(byte* data) {
    auto_interior_illum_ = !getBit(data, 4, 5);
    dash_->setAutoInteriorIllumination(auto_interior_illum_);

    selective_unlock_ = getBit(data, 4, 7);
    dash_->setSelectiveDoorUnlock(selective_unlock_);

    remote_horn_ = !getBit(data, 7, 3);
    dash_->setRemoteKeyResponseHorn(remote_horn_);
}

void VehicleSettings::receiveState21(byte* data) {
    remote_lights_ = (RemoteLights)((data[1] >> 6) & 0x03);
    switch (remote_lights_) {
        case LIGHTS_OFF:
            dash_->setRemoteKeyResponseLights(DashSettingsController::LIGHTS_OFF);
            break;
        case LIGHTS_UNLOCK:
            dash_->setRemoteKeyResponseLights(DashSettingsController::LIGHTS_UNLOCK);
            break;
        case LIGHTS_LOCK:
            dash_->setRemoteKeyResponseLights(DashSettingsController::LIGHTS_LOCK);
            break;
        case LIGHTS_ON:
            dash_->setRemoteKeyResponseLights(DashSettingsController::LIGHTS_ON);
            break;
    }

    relock_time_ = (ReLockTime)((data[1] >> 4) & 0x03);
    switch (relock_time_) {
        case RELOCK_1M:
            dash_->setAutoReLockTime(DashSettingsController::RELOCK_1M);
            break;
        case RELOCK_OFF:
            dash_->setAutoReLockTime(DashSettingsController::RELOCK_OFF);
            break;
        case RELOCK_5M:
            dash_->setAutoReLockTime(DashSettingsController::RELOCK_5M);
            break;
    }

    hl_sens_ = (HeadlightSensitivity)((data[2] >> 2) & 0x03);
    switch (hl_sens_) {
        case HL_SENS_1:
            dash_->setAutoHeadlightSensitivity(0);
            break;
        case HL_SENS_2:
            dash_->setAutoHeadlightSensitivity(1);
            break;
        case HL_SENS_3:
            dash_->setAutoHeadlightSensitivity(2);
            break;
        case HL_SENS_4:
            dash_->setAutoHeadlightSensitivity(3);
            break;
    }

    hl_delay_ = (HeadlightOffDelay)(((data[2] & 0x01) << 2) | ((data[3] >> 6) & 0x03));
    switch (hl_delay_) {
        case HL_DELAY_0S:
            dash_->setAutoHeadlightOffDelay(DashSettingsController::DELAY_0S);
            break;
        case HL_DELAY_30S:
            dash_->setAutoHeadlightOffDelay(DashSettingsController::DELAY_30S);
            break;
        case HL_DELAY_45S:
            dash_->setAutoHeadlightOffDelay(DashSettingsController::DELAY_45S);
            break;
        case HL_DELAY_60S:
            dash_->setAutoHeadlightOffDelay(DashSettingsController::DELAY_60S);
            break;
        case HL_DELAY_90S:
            dash_->setAutoHeadlightOffDelay(DashSettingsController::DELAY_90S);
            break;
        case HL_DELAY_120S:
            dash_->setAutoHeadlightOffDelay(DashSettingsController::DELAY_120S);
            break;
        case HL_DELAY_150S:
            dash_->setAutoHeadlightOffDelay(DashSettingsController::DELAY_150S);
            break;
        case HL_DELAY_180S:
            dash_->setAutoHeadlightOffDelay(DashSettingsController::DELAY_180S);
            break;
    }
}

void VehicleSettings::receiveState22(byte* data) {
    speed_wiper_ = !getBit(data, 1, 7);
    dash_->setSpeedSensingWiperInterval(speed_wiper_);
}

void VehicleSettings::push() {
    initE_->loop();
    initF_->loop();
    stateE_->loop();
    stateF_->loop();
    setter_->loop();

    // Request initial state if init is complete.
    if (!init_ && initE_->ready() && initF_->ready()) {
        stateE_->send();
        stateF_->send();
        init_ = true;
    }
}
