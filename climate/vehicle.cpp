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

void VehicleClimate::connect(Connection* can, DashController* dash) {
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
    toggleBit(frame540_, 5, 5);
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
    toggleBit(frame540_, 5, 5);
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
    if (id_ == 0 || op_ != OP_READY) {
        return false;
    }
    return sendRequest(OP_ENTER);
}

bool SettingsCommand::sendControl(Op op, byte prefix0, byte prefix1, byte prefix2, byte value = 0xFF) {
    if (id_ == 0) {
        return;
    }
    buffer_[0] = prefix0;
    buffer_[1] = prefix1;
    buffer_[2] = prefix2;
    buffer_[3] = value;
    memset(buffer_+4, 0xFF, 5);
    if (!conn_->write(id_, 8, buffer_)) {
        return false;
    }
    op_ = op;
    last_write_ = millis();
    INFO_MSG_FRAME("vehicle: write frame ", id_, 8, buffer_);
    return true;
}

bool SettingsCommand::sendRequest(Op op, uint8_t value = 0xFF) {
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
    if (id != (id_ & ~0x010 | 0x020) || len != 8) {
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
            return matchFrame(data, 0x02, 0x7B, 0x01);
        case OP_GET_STATE_71E_2X:
            return matchFrame(data, 0x21, 0xE0, 0x00) ||
                matchFrame(data, 0x22, 0x94, 0x00);
    }
}

bool SettingsCommand::matchAndSend(uint32_t id, uint8_t len, byte* data, Op match, Op send, uint8_t value = 0xFF) {
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
            matchAndSend(id, len, data, OP_ENTER, OP_EXIT);
    }
}

void SettingsUpdate::send(Op setting, uint8_t value) {
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
    SettingsCommand::send();
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

void VehicleSettings::connect(Connection* can) {
    can_ = can;
    if (initE_ != nullptr) {
        delete initE_;
    }
    initE_ = new SettingsInit(can, SettingsCommand::FRAME_E);

    if (initF_ != nullptr) {
        delete initF_;
    }
    initF_ = new SettingsInit(can, SettingsCommand::FRAME_F);

    initE_->send();
    initF_->send();
}

void VehicleSettings::receive(uint32_t id, uint8_t len, byte* data) {
    if ((id != 0x72E && id != 0x72F) || len != 8) {
        return;
    }
    initE_->receive(id, len, data);
    initF_->receive(id, len, data);
}

void VehicleSettings::push() {
    initE_->loop();
    initF_->loop();
}
