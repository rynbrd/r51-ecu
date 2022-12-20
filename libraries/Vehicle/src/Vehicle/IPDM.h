#ifndef _R51_VEHICLE_IPDM_H_
#define _R51_VEHICLE_IPDM_H_

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include <Core.h>
#include <Faker.h>

namespace R51 {

enum class IPDMEvent : uint8_t {
    POWER_STATE = 0x00,
};

// Tracks IPDM state stored in the 0x625 CAN frame.
class IPDM : public Caster::Node<Message> {
    public:
        IPDM(uint32_t tick_ms = 0, Faker::Clock* clock = Faker::Clock::real()) :
            event_((uint8_t)SubSystem::IPDM, (uint8_t)IPDMEvent::POWER_STATE,
                    (uint8_t[]){0x00}), ticker_(tick_ms, tick_ms == 0, clock) {}

        // Handle a 0x625 IPDM state frame. Returns true if the state changed
        // as a result of handling the frame.
        void handle(const Message& msg, const Caster::Yield<Message>& yield) override;

        // Yield a BODY_POWER_STATE frame on change or tick.
        void emit(const Caster::Yield<Message>& yield) override;

    private:
        void yieldEvent(const Caster::Yield<Message>& yield);
        void handleFrame(const Canny::CAN20Frame& frame, const Caster::Yield<Message>& yield);
        void handleEvent(const Event& event, const Caster::Yield<Message>& yield);

        Event event_;
        Ticker ticker_;
};

}  // namespace R51

#endif  // _R51_VEHICLE_IPDM_H_
