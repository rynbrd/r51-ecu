#ifndef _R51_VEHICLE_TIRES_H
#define _R51_VEHICLE_TIRES_H

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include <Common.h>
#include "Config.h"

namespace R51 {

enum class TireEvent : uint8_t {
    PRESSURE_STATE = 0x00,
    SWAP_POSITION = 0x01,
};

// Track tire pressure as reported in the 0x385 CAN frame.
class TirePressureState : public Caster::Node<Message> {
    public:
        TirePressureState(ConfigStore* config = nullptr,
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

}

#endif  // _R51_VEHICLE_TIRES_H
