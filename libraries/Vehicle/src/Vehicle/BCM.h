#ifndef _R51_VEHICLE_BCM_H_
#define _R51_VEHICLE_BCM_H_

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include <Core.h>
#include <Faker.h>
#include "Config.h"
#include "MomentaryOutput.h"

namespace R51 {

enum class BCMEvent : uint8_t {
    ILLUM_STATE         = 0x00, // Dash illumination state.
    TIRE_PRESSURE_STATE = 0x01, // The current tire pressures.

    TOGGLE_DEFROST_CMD  = 0x10, // Toggle defrost on/off.
    TIRE_SWAP_CMD       = 0x11, // Swaps the reported position of two tires.
};

class IllumState : public Event {
    public:
        IllumState() : Event(SubSystem::BCM, (uint8_t)BCMEvent::ILLUM_STATE, {0x00}) {}

        EVENT_PROPERTY(bool, illum, data[0] != 0x00, data[0] = (uint8_t)value);
};

// Emit BCM illumination state. Normally this is output via 12v from the BCM
// directly to the dash. We simulate this by reading headlamp state from the
// IPDM to determine whether dash lights should be illuminated. This avoids the
// need to connect the ECM to the BCM illum+ wire.
class Illum : public Caster::Node<Message> {
    public:
        Illum() {}

        // Handles IPDM power events to determine illumation state.
        void handle(const Message& message, const Caster::Yield<Message>& yield) override;
    private:
        IllumState state_;
};

// Controls the defrost heater via a GPIO pin connected to the BCM. The pin is
// momentarily pulled high to simulate a button press.
class Defrost : public Caster::Node<Message> {
    public:
        Defrost(int output_pin, uint16_t output_ms,
                Faker::Clock* clock = Faker::Clock::real(),
                Faker::GPIO* gpio = Faker::GPIO::real()) :
            output_(output_pin, output_ms, clock, gpio) {}

        // Handles the IPDM TOGGLE_DEFROST_CMD mesage.
        void handle(const Message& message, const Caster::Yield<Message>&) override;

        // Does not emit any messages but required to update the GPIO status.
        void emit(const Caster::Yield<Message>&) override;

    private:
        MomentaryOutput output_;
};

// Track tire pressure as reported in the 0x385 CAN frame.
class TirePressure : public Caster::Node<Message> {
    public:
        TirePressure(ConfigStore* config = nullptr,
                uint32_t tick_ms = 0, Faker::Clock* clock = Faker::Clock::real());

        // Handle 0x385 tire pressure state frames. Returns true if the state
        // changed as a result of handling the frame.
        void handle(const Message& msg, const Caster::Yield<Message>& yield) override;

        // Yield a TIRE_PRESSURE_STATE frame on change or tick.
        void emit(const Caster::Yield<Message>& yield) override;

    private:
        void yieldEvent(const Caster::Yield<Message>& yield);
        void handleFrame(const Canny::CAN20Frame& frame, const Caster::Yield<Message>& yield);
        void handleEvent(const Event& event, const Caster::Yield<Message>& yield);
        void swapPosition(uint8_t a, uint8_t b, const Caster::Yield<Message>& yield);

        ConfigStore* config_;
        Event event_;
        Ticker ticker_;
        uint8_t map_[4];
};

}  // namespace R51

#endif  // _R51_VEHICLE_BCM_H_
