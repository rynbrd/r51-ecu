#include "settings.h"

#include <Faker.h>
#include "binary.h"
#include "bus.h"
#include "config.h"
#include "debug.h"
#include "frame.h"


// Available sequence states. States other than "ready" represent a frame which
// is sent on the bus which requires a specific response.
enum State : uint8_t {
    // Indicates the command is ready to send.
    STATE_READY = 0,

    // Enter and exit settings requests.
    STATE_ENTER,
    STATE_EXIT,

    // Init requests.
    STATE_INIT_00,
    STATE_INIT_20,
    STATE_INIT_40,
    STATE_INIT_60,

    // Settings requests.
    STATE_AUTO_INTERIOR_ILLUM,
    STATE_AUTO_HL_SENS,
    STATE_AUTO_HL_DELAY,
    STATE_SPEED_SENS_WIPER,
    STATE_REMOTE_KEY_HORN,
    STATE_REMOTE_KEY_LIGHT,
    STATE_AUTO_RELOCK_TIME,
    STATE_SELECT_DOOR_UNLOCK,
    STATE_SLIDE_DRIVER_SEAT,
    STATE_RETRIEVE_71E_10,
    STATE_RETRIEVE_71E_2X,
    STATE_RETRIEVE_71F_05,
    STATE_RESET,
};

inline uint32_t responseId(uint32_t request_id) {
    return (request_id & ~0x010) | 0x020;
}


// Fill a settings frame with a payload.
bool fillRequest(Frame* frame, uint32_t id, byte prefix0, byte prefix1, byte prefix2, uint8_t value = 0xFF) {
    frame->id = id;
    frame->len = 8;
    frame->data[0] = prefix0;
    frame->data[1] = prefix1;
    frame->data[2] = prefix2;
    frame->data[3] = value;
    memset(frame->data+4, 0xFF, 4);
    return true;
}

// Fill a settings frame with data to be sent when the sequence transitions to
// the given state. Some state transitions require value be attached.
bool fillRequest(Frame* frame, uint32_t id, uint8_t state, uint8_t value = 0xFF) {
    switch (state) {
        case STATE_READY:
            return false;
        case STATE_ENTER:
            return fillRequest(frame, id, 0x02, 0x10, 0xC0);
        case STATE_EXIT:
            return fillRequest(frame, id, 0x02, 0x10, 0x81);
        case STATE_INIT_00:
            return fillRequest(frame, id, 0x02, 0x3B, 0x00);
        case STATE_INIT_20:
            return fillRequest(frame, id, 0x02, 0x3B, 0x20);
        case STATE_INIT_40:
            return fillRequest(frame, id, 0x02, 0x3B, 0x40);
        case STATE_INIT_60:
            return fillRequest(frame, id, 0x02, 0x3B, 0x60);
        case STATE_AUTO_INTERIOR_ILLUM:
            return fillRequest(frame, id, 0x03, 0x3B, 0x10, value);
        case STATE_AUTO_HL_SENS:
            return fillRequest(frame, id, 0x03, 0x3B, 0x37, value);
        case STATE_AUTO_HL_DELAY:
            return fillRequest(frame, id, 0x03, 0x3B, 0x39, value);
        case STATE_SPEED_SENS_WIPER:
            return fillRequest(frame, id, 0x03, 0x3B, 0x47, value);
        case STATE_REMOTE_KEY_HORN:
            return fillRequest(frame, id, 0x03, 0x3B, 0x2A, value);
        case STATE_REMOTE_KEY_LIGHT:
            return fillRequest(frame, id, 0x03, 0x3B, 0x2E, value);
        case STATE_AUTO_RELOCK_TIME:
            return fillRequest(frame, id, 0x03, 0x3B, 0x2F, value);
        case STATE_SELECT_DOOR_UNLOCK:
            return fillRequest(frame, id, 0x03, 0x3B, 0x02, value);
        case STATE_SLIDE_DRIVER_SEAT:
            return fillRequest(frame, id, 0x03, 0x3B, 0x01, value);
        case STATE_RETRIEVE_71E_10:
            return fillRequest(frame, id, 0x02, 0x21, 0x01);
        case STATE_RETRIEVE_71E_2X:
            return fillRequest(frame, id, 0x30, 0x00, 0x0A);
        case STATE_RETRIEVE_71F_05:
            return fillRequest(frame, id, 0x02, 0x21, 0x01);
        case STATE_RESET:
            return fillRequest(frame, id, 0x03, 0x3B, 0x1F, 0x00);
        default:
            ERROR_MSG_VAL("settings: fill unsupported state ", state);
            return false;
    }
}

// Match the frame against the given byte prefix.
inline bool matchPrefix(const byte* data, byte prefix0) {
    return data[0] == prefix0;
}

// Match the frame against the given byte prefix.
inline bool matchPrefix(const byte* data, byte prefix0, byte prefix1, byte prefix2) {
    return data[0] == prefix0 && data[1] == prefix1 && data[2] == prefix2;
}

// Return true if the frame matches the given state.
bool matchState(const byte* data, uint8_t state) {
    switch (state) {
        case STATE_READY:
            return false;
        case STATE_ENTER:
            return matchPrefix(data, 0x02, 0x50, 0xC0);
        case STATE_EXIT:
            return matchPrefix(data, 0x02, 0x50, 0x81);
        case STATE_INIT_00:
            return matchPrefix(data, 0x06, 0x7B, 0x00);
        case STATE_INIT_20:
            return matchPrefix(data, 0x06, 0x7B, 0x20);
        case STATE_INIT_40:
            return matchPrefix(data, 0x06, 0x7B, 0x40);
        case STATE_INIT_60:
            return matchPrefix(data, 0x06, 0x7B, 0x60);
        case STATE_AUTO_INTERIOR_ILLUM:
            return matchPrefix(data, 0x02, 0x7B, 0x10);
        case STATE_AUTO_HL_SENS:
            return matchPrefix(data, 0x02, 0x7B, 0x37);
        case STATE_AUTO_HL_DELAY:
            return matchPrefix(data, 0x02, 0x7B, 0x39);
        case STATE_SPEED_SENS_WIPER:
            return matchPrefix(data, 0x02, 0x7B, 0x47);
        case STATE_REMOTE_KEY_HORN:
            return matchPrefix(data, 0x02, 0x7B, 0x2A);
        case STATE_REMOTE_KEY_LIGHT:
            return matchPrefix(data, 0x02, 0x7B, 0x2E);
        case STATE_AUTO_RELOCK_TIME:
            return matchPrefix(data, 0x02, 0x7B, 0x2F);
        case STATE_SELECT_DOOR_UNLOCK:
            return matchPrefix(data, 0x02, 0x7B, 0x02);
        case STATE_SLIDE_DRIVER_SEAT:
            return matchPrefix(data, 0x02, 0x7B, 0x01);
        case STATE_RETRIEVE_71E_10:
            return matchPrefix(data, 0x10);
        case STATE_RETRIEVE_71E_2X:
            return matchPrefix(data, 0x21) || matchPrefix(data, 0x22);
        case STATE_RETRIEVE_71F_05:
            return matchPrefix(data, 0x05);
        case STATE_RESET:
            return matchPrefix(data, 0x02, 0x7B, 0x1F);
        default:
            ERROR_MSG_VAL("settings: match unsupported state ", state);
            return false;
    }
}

bool SettingsSequence::trigger() {
    if (state_ != STATE_READY) {
        return false;
    }
    started_ = clock_->millis();
    state_ = STATE_ENTER;
    sent_ = false;
    return true;
}

bool SettingsSequence::ready() const {
    return state_ == STATE_READY;
}

bool SettingsSequence::receive(Frame* frame) {
    if (clock_->millis() - started_ >= SETTINGS_RESPONSE_TIMEOUT) {
        state_ = STATE_READY;
        return false;
    }
    if (state_ == STATE_READY || sent_) {
        return false;
    }
    sent_ = true;
    bool result = fillRequest(frame, request_id_, state_, value_);
    return result;
}

void SettingsSequence::send(const Frame& frame) {
    if (frame.id != responseId(request_id_)) {
        // not destined for this sequence
        ERROR_MSG_FRAME("settings: unrecognized frame: ", frame);
        return;
    }
    if (!matchState(frame.data, state_)) {
        // frame does not match the current state
        return;
    }
    uint8_t nextState = next();
    if (state_ != nextState) {
        state_ = nextState;
        sent_ = false;
    }
}

uint8_t SettingsSequence::next() {
    switch (request_id_) {
        case SETTINGS_FRAME_E:
            return nextE();
        case SETTINGS_FRAME_F:
            return nextF();
        default:
            return STATE_READY;
    }
}

uint8_t SettingsInit::nextE() {
    switch (state()) {
        case STATE_ENTER:
            return STATE_INIT_00;
        case STATE_INIT_00:
            return STATE_INIT_20;
        case STATE_INIT_20:
            return STATE_INIT_40;
        case STATE_INIT_40:
            return STATE_INIT_60;
        case STATE_INIT_60:
            return STATE_EXIT;
        default:
            return STATE_READY;
    }
}

uint8_t SettingsInit::nextF() {
    switch (state()) {
        case STATE_ENTER:
            return STATE_INIT_00;
        case STATE_INIT_00:
            return STATE_EXIT;
        default:
            return STATE_READY;
    }
}

uint8_t SettingsRetrieve::nextE() {
    switch (state()) {
        case STATE_ENTER:
            return STATE_RETRIEVE_71E_10;
        case STATE_RETRIEVE_71E_10:
            state2x_ = false;
            return STATE_RETRIEVE_71E_2X;
        case STATE_RETRIEVE_71E_2X:
            if (state2x_) {
                return STATE_EXIT;
            } else {
                state2x_ = true;
                return STATE_RETRIEVE_71E_2X;
            }
        default:
            return STATE_READY;
    }
}

uint8_t SettingsRetrieve::nextF() {
    switch (state()) {
        case STATE_ENTER:
            return STATE_RETRIEVE_71F_05;
        case STATE_RETRIEVE_71F_05:
            return STATE_EXIT;
        default:
            return STATE_READY;
    }
}

void SettingsUpdate::setPayload(uint8_t update, uint8_t value) {
    update_ = update;
    setValue(value);
}

uint8_t SettingsUpdate::nextE() {
    const uint8_t state = this->state();
    if (state == STATE_ENTER) {
        return update_;
    } else if (state == update_) {
        return STATE_RETRIEVE_71E_10;
    } else if (state == STATE_RETRIEVE_71E_10) {
        state2x_ = false;
        return STATE_RETRIEVE_71E_2X;
    } else if (state == STATE_RETRIEVE_71E_2X) {
        if (state2x_) {
            return STATE_EXIT;
        } else {
            state2x_ = true;
            return STATE_RETRIEVE_71E_2X;
        }
    }
    return STATE_READY;
}

uint8_t SettingsUpdate::nextF() {
    const uint8_t state = this->state();
    if (state == STATE_ENTER) {
        return update_;
    } else if (state == update_) {
        return STATE_RETRIEVE_71F_05;
    } else if (state == STATE_RETRIEVE_71F_05) {
        return STATE_EXIT;
    }
    return STATE_READY;
}

uint8_t SettingsReset::nextE() {
    switch (state()) {
        case STATE_ENTER:
            return STATE_RESET;
        case STATE_RESET:
            return STATE_RETRIEVE_71E_10;
        case STATE_RETRIEVE_71E_10:
            state2x_ = false;
            return STATE_RETRIEVE_71E_2X;
        case STATE_RETRIEVE_71E_2X:
            if (state2x_) {
                return STATE_EXIT;
            } else {
                state2x_ = true;
                return STATE_RETRIEVE_71E_2X;
            }
        default:
            return STATE_READY;
    }
}

uint8_t SettingsReset::nextF() {
    switch (state()) {
        case STATE_ENTER:
            return STATE_RESET;
        case STATE_RESET:
            return STATE_RETRIEVE_71F_05;
        case STATE_RETRIEVE_71F_05:
            return STATE_EXIT;
        default:
            return STATE_READY;
    }
}

Settings::Settings(Faker::Clock* clock) :
        initE_(SETTINGS_FRAME_E, clock), retrieveE_(SETTINGS_FRAME_E, clock),
        updateE_(SETTINGS_FRAME_E, clock), resetE_(SETTINGS_FRAME_E, clock),
        initF_(SETTINGS_FRAME_F, clock), retrieveF_(SETTINGS_FRAME_F, clock),
        updateF_(SETTINGS_FRAME_F, clock), resetF_(SETTINGS_FRAME_F, clock),
        clock_(clock), state_changed_(false), state_last_broadcast_(0) {
    initFrame(&state_, SETTINGS_STATE_FRAME_ID, 8);
    memset(control_state_, 0, 8);
}

void Settings::receive(const Broadcast& broadcast) {
    if (initE_.receive(&buffer_)) {
        broadcast(buffer_);
    } else if (retrieveE_.receive(&buffer_)) {
        broadcast(buffer_);
    } else if (updateE_.receive(&buffer_)) {
        broadcast(buffer_);
    } else if (resetE_.receive(&buffer_)) {
        broadcast(buffer_);
    }

    if (initF_.receive(&buffer_)) {
        broadcast(buffer_);
    } else if (retrieveF_.receive(&buffer_)) {
        broadcast(buffer_);
    } else if (updateF_.receive(&buffer_)) {
        broadcast(buffer_);
    } else if (resetF_.receive(&buffer_)) {
        broadcast(buffer_);
    }

    if (state_changed_ || clock_->millis() - state_last_broadcast_ >= SETTINGS_STATE_FRAME_HB) {
        broadcast(state_);
    }
}

void Settings::send(const Frame& frame) {
    if (frame.len < 8) {
        return;
    }
    if (frame.id == responseId(SETTINGS_FRAME_E)) {
        initE_.send(frame);
        retrieveE_.send(frame);
        updateE_.send(frame);
        resetE_.send(frame);
        handleState(frame);
    } else if (frame.id == responseId(SETTINGS_FRAME_F)) {
        initF_.send(frame);
        retrieveF_.send(frame);
        updateF_.send(frame);
        resetF_.send(frame);
        handleState(frame);
    } else if (frame.id == SETTINGS_CONTROL_FRAME_ID) {
        handleControl(frame);
    }
}

void Settings::handleState(const Frame& frame) {
    if (matchPrefix(frame.data, 0x05)) {
        handleState05(frame.data);
    } else if (matchPrefix(frame.data, 0x10)) {
        handleState10(frame.data);
    } else if (matchPrefix(frame.data, 0x21)) {
        handleState21(frame.data);
    } else if (matchPrefix(frame.data, 0x22)) {
        handleState22(frame.data);
    }
}

void Settings::handleState05(const byte* data) {
    setSlideDriverSeatBackOnExit(getBit(data, 3, 0));
}

void Settings::handleState10(const byte* data) {
    setAutoInteriorIllumination(getBit(data, 4, 5));
    setSelectiveDoorUnlock(getBit(data, 4, 7));
    setRemoteKeyResponseHorn(getBit(data, 7, 3));
}

void Settings::handleState21(const byte* data) {
    // Translates incoming state to our owns tate representation. A 0 value
    // typically represents the default on the BCM side.

    switch ((data[1] >> 6) & 0x03) {
        case 0x00:
            setRemoteKeyResponseLights(Settings::LIGHTS_OFF);
            break;
        case 0x01:
            setRemoteKeyResponseLights(Settings::LIGHTS_UNLOCK);
            break;
        case 0x02:
            setRemoteKeyResponseLights(Settings::LIGHTS_LOCK);
            break;
        case 0x03:
            setRemoteKeyResponseLights(Settings::LIGHTS_ON);
            break;
    }

    switch ((data[1] >> 4) & 0x03) {
        case 0x00:
            setAutoReLockTime(Settings::RELOCK_1M);
            break;
        case 0x01:
            setAutoReLockTime(Settings::RELOCK_OFF);
            break;
        case 0x02:
            setAutoReLockTime(Settings::RELOCK_5M);
            break;
    }

    switch ((data[2] >> 2) & 0x03) {
        case 0x03:
            setAutoHeadlightSensitivity(0);
            break;
        case 0x00:
            setAutoHeadlightSensitivity(1);
            break;
        case 0x01:
            setAutoHeadlightSensitivity(2);
            break;
        case 0x02:
            setAutoHeadlightSensitivity(3);
            break;
    }

    switch (((data[2] & 0x01) << 2) | ((data[3] >> 6) & 0x03)) {
        case 0x01:
            setAutoHeadlightOffDelay(Settings::DELAY_0S);
            break;
        case 0x02:
            setAutoHeadlightOffDelay(Settings::DELAY_30S);
            break;
        case 0x00:
            setAutoHeadlightOffDelay(Settings::DELAY_45S);
            break;
        case 0x03:
            setAutoHeadlightOffDelay(Settings::DELAY_60S);
            break;
        case 0x04:
            setAutoHeadlightOffDelay(Settings::DELAY_90S);
            break;
        case 0x05:
            setAutoHeadlightOffDelay(Settings::DELAY_120S);
            break;
        case 0x06:
            setAutoHeadlightOffDelay(Settings::DELAY_150S);
            break;
        case 0x07:
            setAutoHeadlightOffDelay(Settings::DELAY_180S);
            break;
    }
}

void Settings::handleState22(const byte* data) {
    setSpeedSensingWiperInterval(!getBit(data, 1, 7));
}

void Settings::handleControl(const Frame& frame) {
    // check if any bits have flipped
    if (xorBits(control_state_, frame.data, 0, 0)) {
        toggleAutoInteriorIllumination();
    } else if (xorBits(control_state_, frame.data, 0, 1)) {
        toggleSlideDriverSeatBackOnExit();
    } else if (xorBits(control_state_, frame.data, 0, 2)) {
        toggleSpeedSensingWiperInterval();
    } else if (xorBits(control_state_, frame.data, 1, 0)) {
        nextAutoHeadlightSensitivity();
    } else if (xorBits(control_state_, frame.data, 1, 1)) {
        prevAutoHeadlightSensitivity();
    } else if (xorBits(control_state_, frame.data, 1, 4)) {
        nextAutoHeadlightOffDelay();
    } else if (xorBits(control_state_, frame.data, 1, 5)) {
        prevAutoHeadlightOffDelay();
    } else if (xorBits(control_state_, frame.data, 2, 0)) {
        toggleSelectiveDoorUnlock();
    } else if (xorBits(control_state_, frame.data, 2, 4)) {
        nextAutoReLockTime();
    } else if (xorBits(control_state_, frame.data, 2, 5)) {
        prevAutoReLockTime();
    } else if (xorBits(control_state_, frame.data, 3, 0)) {
        toggleRemoteKeyResponseHorn();
    } else if (xorBits(control_state_, frame.data, 3, 2)) {
        nextRemoteKeyResponseLights();
    } else if (xorBits(control_state_, frame.data, 3, 3)) {
        prevRemoteKeyResponseLights();
    } else if (xorBits(control_state_, frame.data, 7, 0)) {
        retrieveSettings();
    } else if (xorBits(control_state_, frame.data, 7, 7)) {
        resetSettingsToDefault();
    }

    // update the stored control state
    memcpy(control_state_, frame.data, 8);
}

bool Settings::filter(const Frame& frame) const {
    return frame.id == SETTINGS_CONTROL_FRAME_ID ||
        frame.id == responseId(SETTINGS_FRAME_E) ||
        frame.id == responseId(SETTINGS_FRAME_F);
}

bool Settings::init() {
    if (!readyE() || !readyF()) {
        return false;
    }
    return initE_.trigger() && initF_.trigger();
}

bool Settings::readyE() const {
    return initE_.ready() && retrieveE_.ready() &&
        updateE_.ready() && resetE_.ready();
}

bool Settings::readyF() const {
    return initF_.ready() && retrieveF_.ready() &&
        updateF_.ready() && resetF_.ready();
}

bool Settings::getAutoInteriorIllumination() const {
    return getBit(state_.data, 0, 0);
}

void Settings::setAutoInteriorIllumination(bool value) {
    setBit(state_.data, 0, 0, value);
    state_changed_ = true;
}

uint8_t Settings::getAutoHeadlightSensitivity() const {
    return state_.data[1] & 0x03;
}

void Settings::setAutoHeadlightSensitivity(uint8_t value) {
    if (value > 3) {
        value = 3;
    }
    state_.data[1] &= 0xFC;
    state_.data[1] |= value;
    state_changed_ = true;
}

Settings::AutoHeadlightOffDelay Settings::getAutoHeadlightOffDelay() const {
    return (AutoHeadlightOffDelay)((state_.data[1] >> 4) & 0x0F);
}

void Settings::setAutoHeadlightOffDelay(Settings::AutoHeadlightOffDelay value) {
    state_.data[1] &= 0x0F;
    state_.data[1] |= value << 4;
    state_changed_ = true;
}

bool Settings::getSpeedSensingWiperInterval() const {
    return getBit(state_.data, 0, 2);
}

void Settings::setSpeedSensingWiperInterval(bool value) {
    setBit(state_.data, 0, 2, value);
    state_changed_ = true;
}

bool Settings::getRemoteKeyResponseHorn() const {
    return getBit(state_.data, 3, 0);
}

void Settings::setRemoteKeyResponseHorn(bool value) {
    setBit(state_.data, 3, 0, value);
    state_changed_ = true;
}

Settings::RemoteKeyResponseLights Settings::getRemoteKeyResponseLights() const {
    return (RemoteKeyResponseLights)((state_.data[3] >> 2) & 0x03);
}

void Settings::setRemoteKeyResponseLights(Settings::RemoteKeyResponseLights value) {
    state_.data[3] &= 0xF3;
    state_.data[3] |= value << 2;
    state_changed_ = true;
}

Settings::AutoReLockTime Settings::getAutoReLockTime() const {
    return (AutoReLockTime)((state_.data[2] >> 4) & 0x0F);
}

void Settings::setAutoReLockTime(AutoReLockTime value) {
    state_.data[2] &= 0x0F;
    state_.data[2] |= value << 4;
    state_changed_ = true;
}

bool Settings::getSelectiveDoorUnlock() const {
    return getBit(state_.data, 2, 0);
}

void Settings::setSelectiveDoorUnlock(bool value) {
    setBit(state_.data, 2, 0, value);
    state_changed_ = true;
}

bool Settings::getSlideDriverSeatBackOnExit() const {
    return getBit(state_.data, 0, 1);
}

void Settings::setSlideDriverSeatBackOnExit(bool value) {
    setBit(state_.data, 0, 1, value);
    state_changed_ = true;
}

bool Settings::toggleAutoInteriorIllumination() {
    if (!readyE()) {
        return false;
    }
    updateE_.setPayload(STATE_AUTO_INTERIOR_ILLUM, !getAutoInteriorIllumination());
    return updateE_.trigger();
}

bool Settings::nextAutoHeadlightSensitivity() {
    return triggerAutoHeadlightSensitivity(getAutoHeadlightSensitivity() + 1);
}

bool Settings::prevAutoHeadlightSensitivity() {
    return triggerAutoHeadlightSensitivity(getAutoHeadlightSensitivity() - 1);
}

bool Settings::triggerAutoHeadlightSensitivity(uint8_t value) {
    if (!readyE() || value > 3) {
        return false;
    }
    switch (value) {
        case 0:
            updateE_.setPayload(STATE_AUTO_HL_SENS, 0x03);
            break;
        case 1:
            updateE_.setPayload(STATE_AUTO_HL_SENS, 0x00);
            break;
        case 2:
            updateE_.setPayload(STATE_AUTO_HL_SENS, 0x01);
            break;
        case 3:
            updateE_.setPayload(STATE_AUTO_HL_SENS, 0x02);
            break;
        default:
            return false;
    }
    return updateE_.trigger();
}

bool Settings::nextAutoHeadlightOffDelay() {
    if (!readyE()) {
        return false;
    }
    switch (getAutoHeadlightOffDelay()) {
        case DELAY_0S:
            updateE_.setPayload(STATE_AUTO_HL_DELAY, 0x02);
            break;
        case DELAY_30S:
            updateE_.setPayload(STATE_AUTO_HL_DELAY, 0x00);
            break;
        case DELAY_45S:
            updateE_.setPayload(STATE_AUTO_HL_DELAY, 0x03);
            break;
        case DELAY_60S:
            updateE_.setPayload(STATE_AUTO_HL_DELAY, 0x04);
            break;
        case DELAY_90S:
            updateE_.setPayload(STATE_AUTO_HL_DELAY, 0x05);
            break;
        case DELAY_120S:
            updateE_.setPayload(STATE_AUTO_HL_DELAY, 0x06);
            break;
        case DELAY_150S:
            updateE_.setPayload(STATE_AUTO_HL_DELAY, 0x07);
            break;
        case DELAY_180S:
        default:
            return false;
    }
    return updateE_.trigger();
}

bool Settings::prevAutoHeadlightOffDelay() {
    if (!readyE()) {
        return false;
    }
    switch (getAutoHeadlightOffDelay()) {
        default:
        case DELAY_0S:
            return false;
        case DELAY_30S:
            updateE_.setPayload(STATE_AUTO_HL_DELAY, 0x01);
            break;
        case DELAY_45S:
            updateE_.setPayload(STATE_AUTO_HL_DELAY, 0x02);
            break;
        case DELAY_60S:
            updateE_.setPayload(STATE_AUTO_HL_DELAY, 0x00);
            break;
        case DELAY_90S:
            updateE_.setPayload(STATE_AUTO_HL_DELAY, 0x03);
            break;
        case DELAY_120S:
            updateE_.setPayload(STATE_AUTO_HL_DELAY, 0x04);
            break;
        case DELAY_150S:
            updateE_.setPayload(STATE_AUTO_HL_DELAY, 0x05);
            break;
        case DELAY_180S:
            updateE_.setPayload(STATE_AUTO_HL_DELAY, 0x06);
            break;
    }
    return updateE_.trigger();
}

bool Settings::toggleSpeedSensingWiperInterval() {
    if (!readyE()) {
        return false;
    }
    updateE_.setPayload(STATE_SPEED_SENS_WIPER, getSpeedSensingWiperInterval());
    return updateE_.trigger();
}

bool Settings::toggleRemoteKeyResponseHorn() {
    if (!readyE()) {
        return false;
    }
    updateE_.setPayload(STATE_REMOTE_KEY_HORN, !getRemoteKeyResponseHorn());
    return updateE_.trigger();
}

bool Settings::nextRemoteKeyResponseLights() {
    return triggerRemoteKeyResponseLights(getRemoteKeyResponseLights() + 1);
}

bool Settings::prevRemoteKeyResponseLights() {
    return triggerRemoteKeyResponseLights(getRemoteKeyResponseLights() - 1);
}

bool Settings::triggerRemoteKeyResponseLights(uint8_t value) {
    if (!readyE() || value > 3) {
        return false;
    }
    updateE_.setPayload(STATE_REMOTE_KEY_LIGHT, value);
    return updateE_.trigger();
}

bool Settings::nextAutoReLockTime() {
    if (!readyE()) {
        return false;
    }
    switch (getAutoReLockTime()) {
        case RELOCK_OFF:
            updateE_.setPayload(STATE_AUTO_RELOCK_TIME, 0x00);
            break;
        case RELOCK_1M:
            updateE_.setPayload(STATE_AUTO_RELOCK_TIME, 0x02);
            break;
        case RELOCK_5M:
        default:
            return false;
    }
    return updateE_.trigger();
}

bool Settings::prevAutoReLockTime() {
    if (!readyE()) {
        return false;
    }
    switch (getAutoReLockTime()) {
        default:
        case RELOCK_OFF:
            return false;
        case RELOCK_1M:
            updateE_.setPayload(STATE_AUTO_RELOCK_TIME, 0x01);
            break;
        case RELOCK_5M:
            updateE_.setPayload(STATE_AUTO_RELOCK_TIME, 0x00);
            break;
    }
    return updateE_.trigger();
}

bool Settings::toggleSelectiveDoorUnlock() {
    if (!readyE()) {
        return false;
    }
    updateE_.setPayload(STATE_SELECT_DOOR_UNLOCK, !getSelectiveDoorUnlock());
    return updateE_.trigger();
}

bool Settings::toggleSlideDriverSeatBackOnExit() {
    if (!readyF()) {
        return false;
    }
    updateF_.setPayload(STATE_SLIDE_DRIVER_SEAT, !getSlideDriverSeatBackOnExit());
    return updateF_.trigger();
}

bool Settings::retrieveSettings() {
    if (!readyE() || !readyF()) {
        return false;
    }
    return retrieveE_.trigger() && retrieveF_.trigger();
}

bool Settings::resetSettingsToDefault() {
    if (!readyE() || !readyF()) {
        return false;
    }
    return resetE_.trigger() && resetF_.trigger();
}
