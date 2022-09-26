#ifndef _R51_CLIMATE_H_
#define _R51_CLIMATE_H_

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include <Common.h>
#include <Faker.h>
#include "ClimateEvents.h"
#include "ClimateFrames.h"

namespace R51 {

// Manages the vehicle climate control system.
class Climate : public Caster::Node<Message> {
    public:
        Climate(uint32_t tick_ms = 0, Faker::Clock* clock = Faker::Clock::real());

        // Update the climate state from vehicle state frames and process
        // control frames.
        void handle(const Message& msg, const Caster::Yield<Message>&) override;

        // Emit control frames to the vehicle and climate state system events.
        void emit(const Caster::Yield<Message>& yield) override;

    private:
        Faker::Clock* clock_;
        uint32_t startup_;
        Ticker state_ticker_;
        Ticker control_ticker_;
        bool state_request_;
        uint8_t state_init_;
        bool control_init_;
        bool temp_state_changed_;
        bool system_state_changed_;
        bool airflow_state_changed_;
        bool system_control_changed_;
        bool fan_control_changed_;
        ClimateTempStateEvent temp_state_;
        ClimateAirflowStateEvent airflow_state_;
        ClimateSystemStateEvent system_state_;
        ClimateSystemControlFrame system_control_;
        ClimateFanControlFrame fan_control_;

        void handleTempFrame(const Canny::Frame& frame);
        void handleSystemFrame(const Canny::Frame& frame);
        void handleEvent(const Event& event);
};

}  // namespace R51

#endif  // __R51_CLIMATE_H__
