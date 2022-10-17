#ifndef _R51_CONTROLS_CONTROLS_H_
#define _R51_CONTROLS_CONTROLS_H_

#include <Arduino.h>
#include <Bluetooth.h>
#include <Caster.h>
#include <Common.h>
#include <Faker.h>
#include <Foundation.h>
#include <Vehicle.h>
#include "Audio.h"
#include "Screen.h"

namespace R51 {

class Controls : public Caster::Node<Message> {
    public:
        Controls() = default;
        virtual ~Controls() = default;

    protected:
        void sendCmd(const Caster::Yield<Message>& yield, AudioEvent cmd);
        void sendCmd(const Caster::Yield<Message>& yield, AudioEvent cmd, uint8_t payload);
        void sendCmd(const Caster::Yield<Message>& yield, AudioEvent cmd, AudioSource payload);
        void sendCmd(const Caster::Yield<Message>& yield, ClimateEvent cmd);
        void sendCmd(const Caster::Yield<Message>& yield, SettingsEvent cmd);
        void sendCmd(const Caster::Yield<Message>& yield, BCMEvent cmd);
        void sendCmd(const Caster::Yield<Message>& yield, BCMEvent cmd, uint8_t payload);
        void sendCmd(const Caster::Yield<Message>& yield, BluetoothEvent cmd);
        void sendCmd(const Caster::Yield<Message>& yield, ScreenEvent cmd);
        void request(const Caster::Yield<Message>& yield, SubSystem subsystem, uint8_t id);

        Event event_;
};

// A button that repeats while it's held.
class RepeatButton {
    public:
        RepeatButton(uint32_t interval, Faker::Clock* clock = Faker::Clock::real()) :
            ticker_(interval, true, clock) {}

        // Press the button.
        void press();

        // Return true if the button's repeat action should be executed.
        bool trigger();

        // Release the button. Returns true if its action should be triggered.
        // This occurs when the button does not exceed its repeat interval.
        bool release();

    private:
        Ticker ticker_;
};

// A button that may perform a separate action on long press.
class LongPressButton {
    public:
        LongPressButton(uint32_t timeout, Faker::Clock* clock = Faker::Clock::real()) :
            clock_(clock), timeout_(timeout), pressed_(0), state_(0) {}

        // Press the button.
        void press();

        // Return true if the button's long press action should be executed.
        bool trigger();

        // Release the button. Returns true if the button's short press action
        // should be executed.
        bool release();

    private:
        Faker::Clock* clock_;
        uint32_t timeout_;
        uint32_t pressed_;
        uint8_t state_;
};

}  // namespace R51

#endif  // _R51_CONTROLS_CONTROLS_H_
