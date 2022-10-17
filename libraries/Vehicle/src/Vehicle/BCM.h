#ifndef _R51_VEHICLE_BCM_H_
#define _R51_VEHICLE_BCM_H_

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include <Common.h>
#include <Faker.h>
#include "Config.h"
#include "MomentaryOutput.h"

namespace R51 {

enum class BCMEvent : uint8_t {
    LIGHTING_STATE      = 0x00, // Interior lighting status.
    TIRE_PRESSURE_STATE = 0x01, // The current tire pressures.

    TOGGLE_DEFROST_CMD  = 0x10, // Toggle defrost on/off.
    TIRE_SWAP_CMD       = 0x11, // Swaps the reported position of two tires.
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
        void handleFrame(const Canny::Frame& frame, const Caster::Yield<Message>& yield);
        void handleEvent(const Event& event, const Caster::Yield<Message>& yield);
        void swapPosition(uint8_t a, uint8_t b, const Caster::Yield<Message>& yield);

        ConfigStore* config_;
        Event event_;
        Ticker ticker_;
        uint8_t map_[4];
};

}  // namespace R51

#endif  // _R51_VEHICLE_BCM_H_
