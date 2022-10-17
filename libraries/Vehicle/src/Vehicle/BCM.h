#ifndef _R51_VEHICLE_BCM_H_
#define _R51_VEHICLE_BCM_H_

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>
#include <Faker.h>
#include "MomentaryOutput.h"

namespace R51 {

enum class BCMEvent : uint8_t {
    TOGGLE_DEFROST_CMD = 0x10,
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

}  // namespace R51

#endif  // _R51_VEHICLE_BCM_H_
