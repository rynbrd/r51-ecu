#include "vehicle.h"

#include <Arduino.h>
#include "binary.h"
#include "connection.h"
#include "dash.h"
#include "debug.h"

// Send buffer for SettingsCommand.
byte buffer_[8];

inline uint8_t clampFan(uint8_t speed) {
    if (speed < 1) {
        speed = 1;
    } else if (speed > 7) {
        speed = 7;
    }
    return speed;
}

inline uint8_t clampTemp(uint8_t temp) {
    if (temp < 60) {
        temp = 60;
    } else if (temp > 90) {
        temp = 90;
    }
    return temp;
}

VehicleClimate::VehicleClimate() {
    can_ = nullptr;
    dash_ = nullptr;

    // Unit has not been initialized.
    init_complete_ = false;
    init_write_count_ = 0;

    // Clear last write so we send an initial frame.
    last_write_ = 0;
    // Send keepalive every 100ms during handshake.
    keepalive_interval_ = 100;

    // Initialize control frames with init data.
    memset(frame540_, 0, 8);
    memset(frame541_, 0, 8);
    frame540_[0] = 0x80;
    frame541_[0] = 0x80;
    frame54x_changed_ = true;

    // Set initial state to "off". We will update state from A/C Auto Amp's
    // initial state frames.
    state_ = STATE_OFF;
    prev_state_ = STATE_OFF;
    ac_ = false;
    dual_ = false;
    recirculate_ = false;
    rear_defrost_ = false;
    mode_ = MODE_FACE;
    fan_speed_ = 0;
    driver_temp_ = 60;
    passenger_temp_ = 60;
}

void VehicleClimate::connect(Connection* can, DashClimateController* dash) {
    can_ = can;
    dash_ = dash;
}

bool VehicleClimate::climateOnline() const {
    return frame540_[0] == 0x60;
}

void VehicleClimate::climateClickOff() {
    if (!toggleFunction(frame540_, 6, 7)) {
        return;
    }

    setState(STATE_OFF);
    updateDash();
}

void VehicleClimate::climateClickAuto() {
    if (!toggleFunction(frame540_, 6, 5)) {
        return;
    }

    switch (state_) {
        case STATE_OFF:
        case STATE_MANUAL:
        case STATE_HALF_MANUAL:
        case STATE_DEFROST:
            setState(STATE_AUTO);
            ac_ = true;
            recirculate_ = false;
            updateDash();
            break;
        case STATE_AUTO:
            break;
    }
}

void VehicleClimate::climateClickAc() {
    if (!toggleFunction(frame540_, 5, 3)) {
        return;
    }

    switch (state_) {
        case STATE_OFF:
        case STATE_HALF_MANUAL:
            break;
        case STATE_DEFROST:
        case STATE_AUTO:
            ac_ = !ac_;
            updateDash();
            break;
        case STATE_MANUAL:
            ac_ = !ac_;
            updateDash();
            break;
    }
}

void VehicleClimate::climateClickDual() {
    if (!toggleFunction(frame540_, 6, 3)) {
        return;
    }

    switch (state_) {
        case STATE_OFF:
        case STATE_DEFROST:
            break;
        case STATE_AUTO:
        case STATE_MANUAL:
        case STATE_HALF_MANUAL:
            dual_ = !dual_;
            updateDash();
            break;
    }
}

void VehicleClimate::climateClickRecirculate() {
    if (!toggleFunction(frame541_, 1, 6)) {
        return;
    }

    switch (state_) {
        case STATE_OFF:
        case STATE_HALF_MANUAL:
            recirculate_ = !recirculate_;
            updateDash();
            break;
        case STATE_AUTO:
        case STATE_MANUAL:
            if (isFaceAirflow()) {
                recirculate_ = !recirculate_;
                updateDash();
            }
            break;
        case STATE_DEFROST:
            break;
    }
}

void VehicleClimate::climateClickMode() {
    if (!toggleFunction(frame540_, 6, 0)) {
        return;
    }

    switch (state_) {
        case STATE_OFF:
            setState(STATE_HALF_MANUAL);
            fan_speed_ = 0;
            break;
        case STATE_AUTO:
            setState(STATE_MANUAL);
            mode_ = cycleMode(mode_);
            if (!isFaceAirflow()) {
                recirculate_ = false;
            }
            break;
        case STATE_MANUAL:
            mode_ = cycleMode(mode_);
            if (!isFaceAirflow()) {
                recirculate_ = false;
            }
            break;
        case STATE_HALF_MANUAL:
            mode_ = cycleMode(mode_);
            break;
        case STATE_DEFROST:
            setState(STATE_MANUAL);
            mode_ = cycleMode(MODE_WINDSHIELD);
            break;
    }
    updateDash();
}

void VehicleClimate::climateClickFrontDefrost() {
    if (!toggleFunction(frame540_, 6, 1)) {
        return;
    }

    switch (state_) {
        case STATE_OFF:
            setState(STATE_DEFROST);
            if (fan_speed_ == 0) {
                fan_speed_ = 3;
            }
            break;
        case STATE_AUTO:
        case STATE_MANUAL:
        case STATE_HALF_MANUAL:
            setState(STATE_DEFROST);
            recirculate_ = false;
            break;
        case STATE_DEFROST:
            if (prev_state_ == STATE_MANUAL) {
                setState(STATE_MANUAL);
            } else {
                setState(STATE_AUTO);
            }
            break;
    }
    updateDash();
}

void VehicleClimate::climateClickRearDefrost() {
    // TODO: implement rear defrost hardware control signal
}

void VehicleClimate::climateClickFanSpeedUp() {
    if (!toggleFunction(frame541_, 0, 5)) {
        return;
    }

    switch (state_) {
        case STATE_OFF:
            setState(STATE_MANUAL);
            fan_speed_ = 1;
            recirculate_ = false;
            break;
        case STATE_AUTO:
            setState(STATE_MANUAL);
            fan_speed_ = clampFan(fan_speed_ + 1);
            break;
        case STATE_HALF_MANUAL:
            setState(STATE_MANUAL);
            fan_speed_ = 1;
            break;
        case STATE_MANUAL:
        case STATE_DEFROST:
            fan_speed_ = clampFan(fan_speed_ + 1);
            break;
    }
    updateDash();
}

void VehicleClimate::climateClickFanSpeedDown() {
    if (!toggleFunction(frame541_, 0, 4)) {
        return;
    }

    switch (state_) {
        case STATE_OFF:
            setState(STATE_MANUAL);
            fan_speed_ = 1;
            recirculate_ = false;
            break;
        case STATE_AUTO:
            setState(STATE_MANUAL);
            fan_speed_ = clampFan(fan_speed_ - 1);
            break;
        case STATE_HALF_MANUAL:
            setState(STATE_MANUAL);
            fan_speed_ = 1;
            break;
        case STATE_MANUAL:
        case STATE_DEFROST:
            fan_speed_ = clampFan(fan_speed_ - 1);
            break;
    }
    updateDash();
}

void VehicleClimate::climateClickDriverTemp(bool increment) {
    if (!climateOnline()) {
        return;
    }

    switch (state_) {
        case STATE_OFF:
            break;
        case STATE_AUTO:
        case STATE_MANUAL:
        case STATE_HALF_MANUAL:
            adjustDriverTemperature(increment, dual_);
            updateDash();
            break;
        case STATE_DEFROST:
            adjustDriverTemperature(increment, false);
            updateDash();
            break;
    }
}

void VehicleClimate::climateClickDriverTempUp() {
    climateClickDriverTemp(true);
}

void VehicleClimate::climateClickDriverTempDown() {
    climateClickDriverTemp(false);
}

void VehicleClimate::climateClickPassengerTemp(bool increment) {
    if (!climateOnline()) {
        return;
    }

    switch (state_) {
        case STATE_OFF:
            break;
        case STATE_DEFROST:
            adjustPassengerTemperature(increment, true);
            break;
        case STATE_AUTO:
        case STATE_MANUAL:
        case STATE_HALF_MANUAL:
            adjustPassengerTemperature(increment, false);
            updateDash();
            break;
    }
}

void VehicleClimate::climateClickPassengerTempUp() {
    climateClickPassengerTemp(true);
}

void VehicleClimate::climateClickPassengerTempDown() {
    climateClickPassengerTemp(false);
}

void VehicleClimate::push() {
    // Send control frames at least once every 200ms to keep the A/C Auto Amp alive.
    if (frame54x_changed_ || millis() - last_write_ >= keepalive_interval_) {
        D(if (frame54x_changed_) {
          INFO_MSG_FRAME("vehicle: send ", 0x540, 8, frame540_);
        })
        if (can_ && !can_->write(0x540, 8, frame540_)) {
            ERROR_MSG("vehicle: failed to send frame 0x540");
            return;
        }
        D(if (frame54x_changed_) {
          INFO_MSG_FRAME("vehicle: send ", 0x541, 8, frame541_);
        })
        if (can_ && !can_->write(0x541, 8, frame541_)) {
            ERROR_MSG("vehicle: failed to send frame 0x541");
            return;
        }

        frame54x_changed_ = false;
        last_write_ = millis();
        init_write_count_++;

        if (!init_complete_ && init_write_count_ >= 4) {
            frame540_[0] = 0x60;
            frame540_[1] = 0x40;
            frame540_[6] = 0x04;
            frame541_[0] = 0x00;
            init_complete_ = true;
            keepalive_interval_ = 200;
        }
    }
}

bool VehicleClimate::isFaceAirflow() const {
    switch (mode_) {
        case MODE_FACE:
        case MODE_AUTO_FACE:
        case MODE_FACE_FEET:
        case MODE_AUTO_FACE_FEET:
            return true;
        default:
            return false;
    }
}

void VehicleClimate::setState(State state) {
    if (state == state_) {
        return;
    }
    prev_state_ = state_;
    state_ = state;
}

uint8_t VehicleClimate::cycleMode(uint8_t mode) {
    switch (mode) {
        case MODE_FACE:
        case MODE_AUTO_FACE:
            return MODE_FACE_FEET;
        case MODE_FACE_FEET:
        case MODE_AUTO_FACE_FEET:
            return MODE_FEET;
        case MODE_FEET:
        case MODE_AUTO_FEET:
            return MODE_FEET_WINDSHIELD;
        case MODE_FEET_WINDSHIELD:
            return MODE_FACE;
        case MODE_WINDSHIELD:
            return MODE_FEET_WINDSHIELD;
        default:
            return MODE_FACE;
    }
}

bool VehicleClimate::toggleFunction(byte* frame, uint8_t offset, uint8_t bit) {
    if (!climateOnline()) {
        return false;
    }
    toggleBit(frame, offset, bit);
    frame54x_changed_ = true;
    return true;
}

bool VehicleClimate::toggleTemperature() {
    return toggleFunction(frame540_, 5, 5);
}

void VehicleClimate::adjustDriverTemperature(bool increment, bool dual) {
    if (!toggleTemperature()) {
        return;
    }

    if (increment) {
        frame540_[3]++;
        driver_temp_++;
    } else {
        frame540_[3]--;
        driver_temp_--;
    }

    if (!dual) {
        if (increment) {
            frame540_[4]++;
        } else {
            frame540_[4]--;
        }
    }
}

void VehicleClimate::adjustPassengerTemperature(bool increment, bool frame_only) {
    if (!toggleTemperature()) {
        return;
    }

    if (increment) {
        frame540_[4]++;
        if (!frame_only) {
            passenger_temp_++;
        }
    } else {
        frame540_[4]--;
        if (!frame_only) {
            passenger_temp_--;
        }
    }
    dual_ = true;
}



void VehicleClimate::receive(uint32_t id, uint8_t len, byte* data) {
    switch(id) {
        case 0x54A:
            receive54A(len, data);
            break;
        case 0x54B:
            receive54B(len, data);
            break;
    }
}

void VehicleClimate::receive54A(uint8_t len, byte* data) {
    if (len != 8) {
        ERROR_MSG_VAL("vehicle: frame 0x54A has invalid length: 8 != ", len);
        return;
    }
    INFO_MSG_FRAME("vehicle: receive ", 0x54A, 8, data);

    uint8_t temp = data[4];
    if (temp != 0) {
        driver_temp_ = clampTemp(temp);
    }
    temp = data[5];
    if (temp != 0 && dual_ && state_ != STATE_DEFROST) {
        passenger_temp_ = clampTemp(temp);
    }
    outside_temp_ = data[7];

    updateDash();
}

void VehicleClimate::receive54B(uint8_t len, byte* data) {
    if (len != 8) {
        ERROR_MSG_VAL("vehicle: frame 0x54B has invalid length: 8 != ", len);
        return;
    }
    INFO_MSG_FRAME("vehicle: receive ", 0x54B, 8, data);

    ac_ = getBit(data, 0, 3);
    dual_ = getBit(data, 3, 7);
    recirculate_ = getBit(data, 3, 4);
    fan_speed_ = (data[2] + 1) / 2;

    if (data[1] != MODE_OFF && data[1] != MODE_WINDSHIELD) {
        mode_ = data[1];
    }

    if (data[1] == MODE_WINDSHIELD) {
        state_ = STATE_DEFROST;
    } else if (getBit(data, 0, 7)) {
        state_ = STATE_OFF;
    } else if (getBit(data, 0, 0)) {
        state_ = STATE_AUTO;
    } else if (fan_speed_ == 0) {
        state_ = STATE_HALF_MANUAL;
    } else {
        state_ = STATE_MANUAL;
    }

    updateDash();
}

void VehicleClimate::updateDash() {
    switch (state_) {
        case STATE_OFF: 
            dash_->setClimateActive(false);
            dash_->setClimateAuto(false);
            dash_->setClimateAc(false);
            dash_->setClimateDual(false);
            dash_->setClimateRecirculate(false);
            dash_->setClimateFrontDefrost(false);
            dash_->setClimateFanSpeed(0);
            dash_->setClimateDriverTemp(0);
            dash_->setClimatePassengerTemp(0);
            updateDashMode(MODE_OFF);
            break;
        case STATE_AUTO:
            dash_->setClimateActive(true);
            dash_->setClimateAuto(true);
            dash_->setClimateAc(ac_);
            dash_->setClimateDual(dual_);
            dash_->setClimateRecirculate(recirculate_);
            dash_->setClimateFrontDefrost(false);
            dash_->setClimateFanSpeed(fan_speed_);
            dash_->setClimateDriverTemp(driver_temp_);
            dash_->setClimatePassengerTemp(dual_ ? passenger_temp_ : driver_temp_);
            updateDashMode(mode_);
            break;
        case STATE_MANUAL:
        case STATE_HALF_MANUAL:
            dash_->setClimateActive(true);
            dash_->setClimateAuto(false);
            dash_->setClimateAc(ac_);
            dash_->setClimateDual(dual_);
            dash_->setClimateRecirculate(recirculate_);
            dash_->setClimateFrontDefrost(false);
            dash_->setClimateFanSpeed(fan_speed_);
            dash_->setClimateDriverTemp(driver_temp_);
            dash_->setClimatePassengerTemp(dual_ ? passenger_temp_ : driver_temp_);
            updateDashMode(mode_);
            break;
        case STATE_DEFROST:
            dash_->setClimateActive(true);
            dash_->setClimateAuto(false);
            dash_->setClimateAc(ac_);
            dash_->setClimateDual(false);
            dash_->setClimateRecirculate(false);
            dash_->setClimateFrontDefrost(true);
            dash_->setClimateFanSpeed(fan_speed_);
            dash_->setClimateDriverTemp(driver_temp_);
            dash_->setClimatePassengerTemp(driver_temp_);
            updateDashMode(MODE_WINDSHIELD);
            break;
    }
}

void VehicleClimate::updateDashMode(uint8_t mode) {
    switch (mode) {
        case MODE_FACE:
        case MODE_AUTO_FACE:
            dash_->setClimateFace(true);
            dash_->setClimateFeet(false);
            dash_->setClimateFrontDefrost(false);
            break;
        case MODE_FACE_FEET:
        case MODE_AUTO_FACE_FEET:
            dash_->setClimateFace(true);
            dash_->setClimateFeet(true);
            dash_->setClimateFrontDefrost(false);
            break;
        case MODE_FEET:
        case MODE_AUTO_FEET:
            dash_->setClimateFace(false);
            dash_->setClimateFeet(true);
            dash_->setClimateFrontDefrost(false);
            break;
        case MODE_FEET_WINDSHIELD:
            dash_->setClimateFace(false);
            dash_->setClimateFeet(true);
            dash_->setClimateFrontDefrost(true);
            break;
        case MODE_WINDSHIELD:
            dash_->setClimateFace(false);
            dash_->setClimateFeet(false);
            dash_->setClimateFrontDefrost(true);
            break;
        default:
            dash_->setClimateFace(false);
            dash_->setClimateFeet(false);
            dash_->setClimateFrontDefrost(false);
            break;
    }
}

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
