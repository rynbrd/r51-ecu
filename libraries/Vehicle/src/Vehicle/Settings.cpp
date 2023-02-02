#include "Settings.h"

#include <Canny.h>
#include <Core.h>
#include <Faker.h>
#include <Foundation.h>

namespace R51 {
namespace {

// Valid frame IDs for settings.
enum SettingsFrameId : uint32_t {
    SETTINGS_FRAME_E = 0x71E,
    SETTINGS_FRAME_F = 0x71F,
};

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
    STATE_AUTO_RELOCK_TIME_CMD,
    STATE_SELECT_DOOR_UNLOCK,
    STATE_SLIDE_DRIVER_SEAT,
    STATE_RETRIEVE_71E_10,
    STATE_RETRIEVE_71E_2X,
    STATE_RETRIEVE_71F_05,
    STATE_RESET,
};

// Available Remote Key Response Lights values.
enum RemoteKeyResponseLights : uint8_t {
    LIGHTS_OFF = 0,
    LIGHTS_UNLOCK = 1,
    LIGHTS_LOCK = 2,
    LIGHTS_ON = 3,
};

// Available Auto Headlight Off Delay values.
enum AutoHeadlightOffDelay : uint8_t {
    DELAY_0S = 0,
    DELAY_30S = 2,
    DELAY_45S = 3,
    DELAY_60S = 4,
    DELAY_90S = 6,
    DELAY_120S = 8,
    DELAY_150S = 10,
    DELAY_180S = 12,
};

// AVailable Auto Re-Lock Time values.
enum AutoReLockTime : uint8_t {
    RELOCK_OFF = 0,
    RELOCK_1M = 1,
    RELOCK_5M = 5,
};

// Return the ID of the response frame for the given settings request frame.
uint32_t responseId(uint32_t request_id) {
    return (request_id & ~0x010) | 0x020;
}

// Fill a settings frame with a payload.
bool fillRequest(Canny::CAN20Frame* frame, uint32_t id, byte prefix0, byte prefix1, byte prefix2, uint8_t value = 0xFF) {
    frame->id(id, 0);
    frame->resize(8);
    frame->data()[0] = prefix0;
    frame->data()[1] = prefix1;
    frame->data()[2] = prefix2;
    frame->data()[3] = value;
    memset(frame->data()+4, 0xFF, 4);
    return true;
}

// Fill a settings frame with data to be sent when the sequence transitions to
// the given state. Some state transitions require value be attached.
bool fillRequest(Canny::CAN20Frame* frame, uint32_t id, uint8_t state, uint8_t value = 0xFF) {
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
        case STATE_AUTO_RELOCK_TIME_CMD:
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
            return false;
    }
}

// Match the frame against the given byte prefix.
bool matchPrefix(const byte* data, byte prefix0) {
    return data[0] == prefix0;
}

// Match the frame against the given byte prefix.
bool matchPrefix(const byte* data, byte prefix0, byte prefix1, byte prefix2) {
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
        case STATE_AUTO_RELOCK_TIME_CMD:
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
            return false;
    }
}

// Return true if auto interior illumination is enabled.
bool getAutoInteriorIllumination(const Event& event) {
    return getBit(event.data, 0, 0);
}

void setAutoInteriorIllumination(Event* event, bool value) {
    setBit(event->data, 0, 0, value);
}

// Return the auto-headlight sensitivity setting. Returns a value from
// 0 to 3 inclusive.
uint8_t getAutoHeadlightSensitivity(const Event& event) {
    return event.data[1] & 0x03;
}

void setAutoHeadlightSensitivity(Event* event, uint8_t value) {
    if (value > 3) {
        value = 3;
    }
    event->data[1] &= 0xFC;
    event->data[1] |= (value & 0x03);
}

// Return the auto-headlight off delay setting.
AutoHeadlightOffDelay getAutoHeadlightOffDelay(const Event& event) {
    return (AutoHeadlightOffDelay)((event.data[1] >> 4) & 0x0F);
}

void setAutoHeadlightOffDelay(Event* event, AutoHeadlightOffDelay value) {
    event->data[1] &= 0x0F;
    event->data[1] |= value << 4;
}

// Return true if speed sensitive wiper interval is enabled.
bool getSpeedSensingWiperInterval(const Event& event) {
    return getBit(event.data, 0, 2);
}

void setSpeedSensingWiperInterval(Event* event, bool value) {
    setBit(event->data, 0, 2, value);
}

// Return true if the remote key response horn is enabled.
bool getRemoteKeyResponseHorn(const Event& event) {
    return getBit(event.data, 3, 0);
}

void setRemoteKeyResponseHorn(Event* event, bool value) {
    setBit(event->data, 3, 0, value);
}

// Return the remote key response lights setting.
RemoteKeyResponseLights getRemoteKeyResponseLights(const Event& event) {
    return (RemoteKeyResponseLights)((event.data[3] >> 2) & 0x03);
}

void setRemoteKeyResponseLights(Event* event, RemoteKeyResponseLights value) {
    event->data[3] &= 0xF3;
    event->data[3] |= (value & 0x03) << 2;
}

// Return the auto re-lock time setting.
AutoReLockTime getAutoReLockTime(const Event& event) {
    return (AutoReLockTime)((event.data[2] >> 4) & 0x0F);
}

void setAutoReLockTime(Event* event, AutoReLockTime value) {
    event->data[2] &= 0x0F;
    event->data[2] |= value << 4;
}

// Return true if selective door unlock is enabled.
bool getSelectiveDoorUnlock(const Event& event) {
    return getBit(event.data, 2, 0);
}

void setSelectiveDoorUnlock(Event* event, bool value) {
    setBit(event->data, 2, 0, value);
}

// Return true if the "slide driver seat back on exit" setting is enabled.
bool getSlideDriverSeatBackOnExit(const Event& event) {
    return getBit(event.data, 0, 1);
}

void setSlideDriverSeatBackOnExit(Event* event, bool value) {
    setBit(event->data, 0, 1, value);
}

}  // namespace

// Send a sequence of frames for managing settings.
class SettingsSequence {
    public:
        // Create a sequence that communicates over the given frame ID.
        SettingsSequence(SettingsFrameId id, Faker::Clock* clock = Faker::Clock::real()) :
            request_id_((uint32_t)id), clock_(clock), started_(0),
            value_(0xFF), state_(0), sent_(false) {}
        virtual ~SettingsSequence() = default;

        // Trigger the sequence. The next call to receive will broadcast the
        // first frame of the sequence. The sequence expects the next frame to
        // match otherwise it resets.
        bool trigger() {
            if (state_ != STATE_READY) {
                return false;
            }
            started_ = clock_->millis();
            state_ = STATE_ENTER;
            sent_ = false;
            return true;
        }

        // Return true if the sequence is ready to send.
        bool ready() const {
            return state_ == STATE_READY;
        }

        // Read the next outgoing frame in the sequence if available. Return
        // true if the frame should be sent or false otherwise.
        bool read(Canny::CAN20Frame* frame) {
            if (clock_->millis() - started_ >= 500) {
                state_ = STATE_READY;
                return false;
            }
            if (state_ == STATE_READY || sent_) {
                return false;
            }
            sent_ = true;
            return fillRequest(frame, request_id_, state_, value_);
        }

        // Handle the next incoming frame in the sequence. If the frame matches
        // the next expected frame in the sequence then the sequence advances
        // to the next state and read will fill the next outgoing frame.
        // Otherwise the sequence resets and becomes ready.
        void handle(const Canny::CAN20Frame& frame) {
            if (frame.id() != responseId(request_id_)) {
                // not destined for this sequence
                return;
            }
            if (!matchState(frame.data(), state_)) {
                // frame does not match the current state
                return;
            }
            uint8_t nextState = next();
            if (state_ != nextState) {
                state_ = nextState;
                sent_ = false;
            }
        }

    protected:
        // The sequence's request ID.
        uint32_t requestId() const { return request_id_; }

        // The sequence's current state.
        uint8_t state() const { return state_; }

        // Set the value to send with the state frames that require value.
        void setValue(uint8_t value) { value_ = value; }

        // Return the next state. If the returned state matches the incoming
        // rame then the sequence transitions to the new state and a frame for
        // the state is sent. If this returns the no state transition occurs
        // and no frame is sent.
        virtual uint8_t nextE() = 0;
        virtual uint8_t nextF() = 0;
    private:
        const uint32_t request_id_;
        Faker::Clock* clock_;
        uint32_t started_;
        uint8_t value_;
        uint8_t state_;
        bool sent_;

        uint8_t next() {
            switch (request_id_) {
                case SETTINGS_FRAME_E:
                    return nextE();
                case SETTINGS_FRAME_F:
                    return nextF();
                default:
                    return STATE_READY;
            }
        }
};

// Sequence to initialize communication with the BCM.
class SettingsInit : public SettingsSequence {
    public:
        SettingsInit(SettingsFrameId id, Faker::Clock* clock = Faker::Clock::real()) :
            SettingsSequence(id, clock) {}
    protected:
        uint8_t nextE() override {
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

        uint8_t nextF() override {
            switch (state()) {
                case STATE_ENTER:
                    return STATE_INIT_00;
                case STATE_INIT_00:
                    return STATE_EXIT;
                default:
                    return STATE_READY;
            }
        }
};

// Sequence used to retrieve settings from the BCM.
class SettingsRetrieve : public SettingsSequence {
    public:
        SettingsRetrieve(SettingsFrameId id, Faker::Clock* clock = Faker::Clock::real()) :
            SettingsSequence(id, clock), state2x_(false) {}
    protected:
        uint8_t nextE() override {
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

        uint8_t nextF() override {
            switch (state()) {
                case STATE_ENTER:
                    return STATE_RETRIEVE_71F_05;
                case STATE_RETRIEVE_71F_05:
                    return STATE_EXIT;
                default:
                    return STATE_READY;
            }
        }

    private:
        bool state2x_;
};

// Sequence used to update a setting in the BCM.
class SettingsUpdate : public SettingsSequence {
    public:
        SettingsUpdate(SettingsFrameId id, Faker::Clock* clock = Faker::Clock::real()) :
            SettingsSequence(id, clock), update_(0), state2x_(false) {}

        // Set the item to update and its value. 
        void setPayload(uint8_t update, uint8_t value) {
            update_ = update;
            setValue(value);
        }
    protected:
        uint8_t nextE() override {
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

        uint8_t nextF() override {
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

    private:
        uint8_t update_;
        bool state2x_;
};

// Sequence used to reset all settings to factory values.
class SettingsReset : public SettingsSequence {
    public:
        SettingsReset(SettingsFrameId id, Faker::Clock* clock = Faker::Clock::real()) :
            SettingsSequence(id, clock), state2x_(false) {}
    protected:
        uint8_t nextE() override {
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

        uint8_t nextF() override {
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

    private:
        bool state2x_;
};

Settings::Settings(Faker::Clock* clock) :
        initE_(new SettingsInit(SETTINGS_FRAME_E, clock)),
        retrieveE_(new SettingsRetrieve(SETTINGS_FRAME_E, clock)),
        updateE_(new SettingsUpdate(SETTINGS_FRAME_E, clock)),
        resetE_(new SettingsReset(SETTINGS_FRAME_E, clock)),
        initF_(new SettingsInit(SETTINGS_FRAME_F, clock)),
        retrieveF_(new SettingsRetrieve(SETTINGS_FRAME_F, clock)),
        updateF_(new SettingsUpdate(SETTINGS_FRAME_F, clock)),
        resetF_(new SettingsReset(SETTINGS_FRAME_F, clock)),
        available_(false), frame_(0, 0, 8),
        event_((uint8_t)SubSystem::SETTINGS, (uint8_t)SettingsEvent::STATE, (uint8_t[]){0x00, 0x00, 0x00, 0x00}) {
}

Settings::~Settings() {
        delete initE_;
        delete retrieveE_;
        delete updateE_;
        delete resetE_;
        delete initF_;
        delete retrieveF_;
        delete updateF_;
        delete resetF_;
}

void Settings::handle(const Message& msg, const Caster::Yield<Message>&) {
    switch (msg.type()) {
        case Message::CAN_FRAME:
            handleFrame(*msg.can_frame());
            break;
        case Message::EVENT:
            handleEvent(*msg.event());
            break;
        default:
            break;
    }
}

void Settings::handleEvent(const Event& event) {
    if (RequestCommand::match(event, SubSystem::SETTINGS,
            (uint8_t)SettingsEvent::STATE)) {
        requestCurrent();
    }
    if (event.subsystem != (uint8_t)SubSystem::SETTINGS) {
        return;
    }

    switch ((SettingsEvent)event.id) {
        case SettingsEvent::TOGGLE_AUTO_INTERIOR_ILLUM_CMD:
            toggleAutoInteriorIllumination();
            break;
        case SettingsEvent::TOGGLE_SLIDE_DRIVER_SEAT_CMD:
            toggleSlideDriverSeatBackOnExit();
            break;
        case SettingsEvent::TOGGLE_SPEED_SENSING_WIPER_CMD:
            toggleSpeedSensingWiperInterval();
            break;
        case SettingsEvent::NEXT_AUTO_HEADLIGHT_SENS_CMD:
            nextAutoHeadlightSensitivity();
            break;
        case SettingsEvent::PREV_AUTO_HEADLIGHT_SENS_CMD:
            prevAutoHeadlightSensitivity();
            break;
        case SettingsEvent::NEXT_AUTO_HEADLIGHT_OFF_DELAY_CMD:
            nextAutoHeadlightOffDelay();
            break;
        case SettingsEvent::PREV_AUTO_HEADLIGHT_OFF_DELAY_CMD:
            prevAutoHeadlightOffDelay();
            break;
        case SettingsEvent::TOGGLE_SELECTIVE_DOOR_UNLOCK_CMD:
            toggleSelectiveDoorUnlock();
            break;
        case SettingsEvent::NEXT_AUTO_RELOCK_TIME_CMD:
            nextAutoReLockTime();
            break;
        case SettingsEvent::PREV_AUTO_RELOCK_TIME_CMD:
            prevAutoReLockTime();
            break;
        case SettingsEvent::TOGGLE_REMOTE_KEY_RESP_HORN_CMD:
            toggleRemoteKeyResponseHorn();
            break;
        case SettingsEvent::NEXT_REMOTE_KEY_RESP_LIGHTS_CMD:
            nextRemoteKeyResponseLights();
            break;
        case SettingsEvent::PREV_REMOTE_KEY_RESP_LIGHTS_CMD:
            prevRemoteKeyResponseLights();
            break;
        case SettingsEvent::FACTORY_RESET_CMD:
            resetSettingsToDefault();
            break;
        default:
            break;
    }
}

void Settings::handleFrame(const Canny::CAN20Frame& frame) {
    if (frame.size() < 8) {
        return;
    }
    if (frame.id() == responseId(SETTINGS_FRAME_E)) {
        initE_->handle(frame);
        retrieveE_->handle(frame);
        updateE_->handle(frame);
        resetE_->handle(frame);
        handleState(frame.data());
    } else if (frame.id() == responseId(SETTINGS_FRAME_F)) {
        initF_->handle(frame);
        retrieveF_->handle(frame);
        updateF_->handle(frame);
        resetF_->handle(frame);
        handleState(frame.data());
    }
}

void Settings::handleState(const byte* data) {
    if (matchPrefix(data, 0x05)) {
        handleState05(data);
        available_ = true;
    } else if (matchPrefix(data, 0x10)) {
        handleState10(data);
        available_ = true;
    } else if (matchPrefix(data, 0x21)) {
        handleState21(data);
        available_ = true;
    } else if (matchPrefix(data, 0x22)) {
        handleState22(data);
        available_ = true;
    }
}

void Settings::handleState05(const byte* data) {
    setSlideDriverSeatBackOnExit(&event_, getBit(data, 3, 0));
}

void Settings::handleState10(const byte* data) {
    setAutoInteriorIllumination(&event_, getBit(data, 4, 5));
    setSelectiveDoorUnlock(&event_, getBit(data, 4, 7));
    setRemoteKeyResponseHorn(&event_, getBit(data, 7, 3));
}

void Settings::handleState21(const byte* data) {
    // Translates incoming state to our own state representation. A 0 value
    // typically represents the default on the BCM side.

    switch ((data[1] >> 6) & 0x03) {
        case 0x00:
            setRemoteKeyResponseLights(&event_, LIGHTS_OFF);
            break;
        case 0x01:
            setRemoteKeyResponseLights(&event_, LIGHTS_UNLOCK);
            break;
        case 0x02:
            setRemoteKeyResponseLights(&event_, LIGHTS_LOCK);
            break;
        case 0x03:
            setRemoteKeyResponseLights(&event_, LIGHTS_ON);
            break;
    }

    switch ((data[1] >> 4) & 0x03) {
        case 0x00:
            setAutoReLockTime(&event_, RELOCK_1M);
            break;
        case 0x01:
            setAutoReLockTime(&event_, RELOCK_OFF);
            break;
        case 0x02:
            setAutoReLockTime(&event_, RELOCK_5M);
            break;
    }

    switch ((data[2] >> 2) & 0x03) {
        case 0x03:
            setAutoHeadlightSensitivity(&event_, 0);
            break;
        case 0x00:
            setAutoHeadlightSensitivity(&event_, 1);
            break;
        case 0x01:
            setAutoHeadlightSensitivity(&event_, 2);
            break;
        case 0x02:
            setAutoHeadlightSensitivity(&event_, 3);
            break;
    }

    switch (((data[2] & 0x01) << 2) | ((data[3] >> 6) & 0x03)) {
        case 0x01:
            setAutoHeadlightOffDelay(&event_, DELAY_0S);
            break;
        case 0x02:
            setAutoHeadlightOffDelay(&event_, DELAY_30S);
            break;
        case 0x00:
            setAutoHeadlightOffDelay(&event_, DELAY_45S);
            break;
        case 0x03:
            setAutoHeadlightOffDelay(&event_, DELAY_60S);
            break;
        case 0x04:
            setAutoHeadlightOffDelay(&event_, DELAY_90S);
            break;
        case 0x05:
            setAutoHeadlightOffDelay(&event_, DELAY_120S);
            break;
        case 0x06:
            setAutoHeadlightOffDelay(&event_, DELAY_150S);
            break;
        case 0x07:
            setAutoHeadlightOffDelay(&event_, DELAY_180S);
            break;
    }
}

void Settings::handleState22(const byte* data) {
    setSpeedSensingWiperInterval(&event_, !getBit(data, 1, 7));
}

void Settings::emit(const Caster::Yield<Message>& yield) {
    if (initE_->read(&frame_)) {
        yield(MessageView(&frame_));
    }
    if (retrieveE_->read(&frame_)) {
        yield(MessageView(&frame_));
    }
    if (updateE_->read(&frame_)) {
        yield(MessageView(&frame_));
    }
    if (resetE_->read(&frame_)) {
        yield(MessageView(&frame_));
    }
    if (initF_->read(&frame_)) {
        yield(MessageView(&frame_));
    }
    if (retrieveF_->read(&frame_)) {
        yield(MessageView(&frame_));
    }
    if (updateF_->read(&frame_)) {
        yield(MessageView(&frame_));
    }
    if (resetF_->read(&frame_)) {
        yield(MessageView(&frame_));
    }
    if (ready() && available_) {
        available_ = false;
        yield(MessageView(&event_));
    }
}

void Settings::init(const Caster::Yield<Message>&) {
    initE_->trigger();
    initF_->trigger();
}

bool Settings::readyE() const {
    return initE_->ready() && retrieveE_->ready() &&
        updateE_->ready() && resetE_->ready();
}

bool Settings::readyF() const {
    return initF_->ready() && retrieveF_->ready() &&
        updateF_->ready() && resetF_->ready();
}

bool Settings::ready() const {
    return readyE() && readyF();
}

bool Settings::toggleAutoInteriorIllumination() {
    if (!readyE()) {
        return false;
    }
    updateE_->setPayload(STATE_AUTO_INTERIOR_ILLUM, !getAutoInteriorIllumination(event_));
    return updateE_->trigger();
}

bool Settings::nextAutoHeadlightSensitivity() {
    return triggerAutoHeadlightSensitivity(getAutoHeadlightSensitivity(event_) + 1);
}

bool Settings::prevAutoHeadlightSensitivity() {
    return triggerAutoHeadlightSensitivity(getAutoHeadlightSensitivity(event_) - 1);
}

bool Settings::triggerAutoHeadlightSensitivity(uint8_t value) {
    if (!readyE() || value > 3) {
        return false;
    }
    switch (value) {
        case 0:
            updateE_->setPayload(STATE_AUTO_HL_SENS, 0x03);
            break;
        case 1:
            updateE_->setPayload(STATE_AUTO_HL_SENS, 0x00);
            break;
        case 2:
            updateE_->setPayload(STATE_AUTO_HL_SENS, 0x01);
            break;
        case 3:
            updateE_->setPayload(STATE_AUTO_HL_SENS, 0x02);
            break;
        default:
            return false;
    }
    return updateE_->trigger();
}

bool Settings::nextAutoHeadlightOffDelay() {
    if (!readyE()) {
        return false;
    }
    switch (getAutoHeadlightOffDelay(event_)) {
        case DELAY_0S:
            updateE_->setPayload(STATE_AUTO_HL_DELAY, 0x02);
            break;
        case DELAY_30S:
            updateE_->setPayload(STATE_AUTO_HL_DELAY, 0x00);
            break;
        case DELAY_45S:
            updateE_->setPayload(STATE_AUTO_HL_DELAY, 0x03);
            break;
        case DELAY_60S:
            updateE_->setPayload(STATE_AUTO_HL_DELAY, 0x04);
            break;
        case DELAY_90S:
            updateE_->setPayload(STATE_AUTO_HL_DELAY, 0x05);
            break;
        case DELAY_120S:
            updateE_->setPayload(STATE_AUTO_HL_DELAY, 0x06);
            break;
        case DELAY_150S:
            updateE_->setPayload(STATE_AUTO_HL_DELAY, 0x07);
            break;
        case DELAY_180S:
        default:
            return false;
    }
    return updateE_->trigger();
}

bool Settings::prevAutoHeadlightOffDelay() {
    if (!readyE()) {
        return false;
    }
    switch (getAutoHeadlightOffDelay(event_)) {
        default:
        case DELAY_0S:
            return false;
        case DELAY_30S:
            updateE_->setPayload(STATE_AUTO_HL_DELAY, 0x01);
            break;
        case DELAY_45S:
            updateE_->setPayload(STATE_AUTO_HL_DELAY, 0x02);
            break;
        case DELAY_60S:
            updateE_->setPayload(STATE_AUTO_HL_DELAY, 0x00);
            break;
        case DELAY_90S:
            updateE_->setPayload(STATE_AUTO_HL_DELAY, 0x03);
            break;
        case DELAY_120S:
            updateE_->setPayload(STATE_AUTO_HL_DELAY, 0x04);
            break;
        case DELAY_150S:
            updateE_->setPayload(STATE_AUTO_HL_DELAY, 0x05);
            break;
        case DELAY_180S:
            updateE_->setPayload(STATE_AUTO_HL_DELAY, 0x06);
            break;
    }
    return updateE_->trigger();
}

bool Settings::toggleSpeedSensingWiperInterval() {
    if (!readyE()) {
        return false;
    }
    updateE_->setPayload(STATE_SPEED_SENS_WIPER, getSpeedSensingWiperInterval(event_));
    return updateE_->trigger();
}

bool Settings::toggleRemoteKeyResponseHorn() {
    if (!readyE()) {
        return false;
    }
    updateE_->setPayload(STATE_REMOTE_KEY_HORN, !getRemoteKeyResponseHorn(event_));
    return updateE_->trigger();
}

bool Settings::nextRemoteKeyResponseLights() {
    return triggerRemoteKeyResponseLights(getRemoteKeyResponseLights(event_) + 1);
}

bool Settings::prevRemoteKeyResponseLights() {
    return triggerRemoteKeyResponseLights(getRemoteKeyResponseLights(event_) - 1);
}

bool Settings::triggerRemoteKeyResponseLights(uint8_t value) {
    if (!readyE() || value > 3) {
        return false;
    }
    updateE_->setPayload(STATE_REMOTE_KEY_LIGHT, value);
    return updateE_->trigger();
}

bool Settings::nextAutoReLockTime() {
    if (!readyE()) {
        return false;
    }
    switch (getAutoReLockTime(event_)) {
        case RELOCK_OFF:
            updateE_->setPayload(STATE_AUTO_RELOCK_TIME_CMD, 0x00);
            break;
        case RELOCK_1M:
            updateE_->setPayload(STATE_AUTO_RELOCK_TIME_CMD, 0x02);
            break;
        case RELOCK_5M:
        default:
            return false;
    }
    return updateE_->trigger();
}

bool Settings::prevAutoReLockTime() {
    if (!readyE()) {
        return false;
    }
    switch (getAutoReLockTime(event_)) {
        default:
        case RELOCK_OFF:
            return false;
        case RELOCK_1M:
            updateE_->setPayload(STATE_AUTO_RELOCK_TIME_CMD, 0x01);
            break;
        case RELOCK_5M:
            updateE_->setPayload(STATE_AUTO_RELOCK_TIME_CMD, 0x00);
            break;
    }
    return updateE_->trigger();
}

bool Settings::toggleSelectiveDoorUnlock() {
    if (!readyE()) {
        return false;
    }
    updateE_->setPayload(STATE_SELECT_DOOR_UNLOCK, !getSelectiveDoorUnlock(event_));
    return updateE_->trigger();
}

bool Settings::toggleSlideDriverSeatBackOnExit() {
    if (!readyF()) {
        return false;
    }
    updateF_->setPayload(STATE_SLIDE_DRIVER_SEAT, !getSlideDriverSeatBackOnExit(event_));
    return updateF_->trigger();
}

bool Settings::requestCurrent() {
    if (!ready()) {
        return false;
    }
    return retrieveE_->trigger() && retrieveF_->trigger();
}

bool Settings::resetSettingsToDefault() {
    if (!ready()) {
        return false;
    }
    return resetE_->trigger() && resetF_->trigger();
}

}  // namespace R51
