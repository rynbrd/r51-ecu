#ifndef _R51_CONSOLE_CLIMATE_H_
#define _R51_CONSOLE_CLIMATE_H_

#include <Arduino.h>
#include <Caster.h>
#include <Vehicle.h>
#include "Command.h"
#include "Console.h"
#include "Error.h"

namespace R51 {
namespace internal {

class ClimateHelp : public Command {
    public:
        Command* next(char*) override {
            return TooManyArgumentsCommand::get();
        }

        void run(Console* console, char*, const Caster::Yield<Message>&) override {
            console->stream()->println("usage: climate CMD [INC|DEC]");
            console->stream()->println("commands: off, auto, ac, dual, defog, fan, recirculate, mode, dtemp, ptemp");
            console->stream()->println("fan, dtemp, and  ptemp take a inc/dec argument");
        }
};

class ClimateSend : public Command {
    public:
        ClimateSend(const Event& event) : event_(event) {}

        Command* next(char*) override {
            return TooManyArgumentsCommand::get();
        }

        void run(Console*, char*, const Caster::Yield<Message>& yield) override {
            yield(MessageView(&event_));
        }

    private:
        Event event_;
};

class ClimateIncDec : public NotEnoughArgumentsCommand {
    public:
        ClimateIncDec(const Event& inc, const Event& dec) : inc_(inc), dec_(dec) {}

        Command* next(char* arg) override {
            if (strcmp(arg, "inc") == 0 || strcmp(arg, "+") == 0) {
                return &inc_;
            } else if (strcmp(arg, "dec") == 0 || strcmp(arg, "-") == 0) {
                return &dec_;
            }
            return NotFoundCommand::get();
        }

    private:
        ClimateSend inc_;
        ClimateSend dec_;
};

class ClimateCommand : public NotEnoughArgumentsCommand {
    public:
        ClimateCommand() :
            off_(Event(SubSystem::CLIMATE, (uint8_t)ClimateEvent::TURN_OFF_CMD)),
            auto_(Event(SubSystem::CLIMATE, (uint8_t)ClimateEvent::TOGGLE_AUTO_CMD)),
            ac_(Event(SubSystem::CLIMATE, (uint8_t)ClimateEvent::TOGGLE_AC_CMD)),
            dual_(Event(SubSystem::CLIMATE, (uint8_t)ClimateEvent::TOGGLE_DUAL_CMD)),
            defog_(Event(SubSystem::CLIMATE, (uint8_t)ClimateEvent::TOGGLE_DEFOG_CMD)),
            fan_(Event(SubSystem::CLIMATE, (uint8_t)ClimateEvent::INC_FAN_SPEED_CMD),
                 Event(SubSystem::CLIMATE, (uint8_t)ClimateEvent::DEC_FAN_SPEED_CMD)),
            recirc_(Event(SubSystem::CLIMATE, (uint8_t)ClimateEvent::TOGGLE_RECIRCULATE_CMD)),
            mode_(Event(SubSystem::CLIMATE, (uint8_t)ClimateEvent::CYCLE_AIRFLOW_MODE_CMD)),
            dtemp_(Event(SubSystem::CLIMATE, (uint8_t)ClimateEvent::INC_DRIVER_TEMP_CMD),
                   Event(SubSystem::CLIMATE, (uint8_t)ClimateEvent::DEC_DRIVER_TEMP_CMD)),
            ptemp_(Event(SubSystem::CLIMATE, (uint8_t)ClimateEvent::INC_PASSENGER_TEMP_CMD),
                   Event(SubSystem::CLIMATE, (uint8_t)ClimateEvent::DEC_PASSENGER_TEMP_CMD)),
            request_(Event(SubSystem::CONTROLLER, (uint8_t)ControllerEvent::REQUEST_CMD,
                           {(uint8_t)SubSystem::CLIMATE})) {}

        Command* next(char* arg) override {
            if (strcmp(arg, "off") == 0 || strcmp(arg, "o") == 0) {
                return &off_;
            } else if (strcmp(arg, "auto") == 0 || strcmp(arg, "a") == 0) {
                return &auto_;
            } else if (strcmp(arg, "ac") == 0 || strcmp(arg, "c") == 0) {
                return &ac_;
            } else if (strcmp(arg, "dual") == 0 || strcmp(arg, "d") == 0) {
                return &dual_;
            } else if (strcmp(arg, "defog") == 0 || strcmp(arg, "w") == 0) {
                return &defog_;
            } else if (strcmp(arg, "fan") == 0 || strcmp(arg, "f") == 0) {
                return &fan_;
            } else if (strcmp(arg, "recirculate") == 0 || strcmp(arg, "r") == 0) {
                return &recirc_;
            } else if (strcmp(arg, "mode") == 0 || strcmp(arg, "m") == 0) {
                return &mode_;
            } else if (strcmp(arg, "dtemp") == 0 || strcmp(arg, "dt") == 0) {
                return &dtemp_;
            } else if (strcmp(arg, "ptemp") == 0 || strcmp(arg, "pt") == 0) {
                return &ptemp_;
            } else if (strcmp(arg, "request") == 0 || strcmp(arg, "req") == 0) {
                return &request_;
            } else if (strcmp(arg, "help") == 0 || strcmp(arg, "h") == 0) {
                return &help_;
            }
            return NotFoundCommand::get();
        }

    private:
        ClimateSend off_;
        ClimateSend auto_;
        ClimateSend ac_;
        ClimateSend dual_;
        ClimateSend defog_;
        ClimateIncDec fan_;
        ClimateSend recirc_;
        ClimateSend mode_;
        ClimateIncDec dtemp_;
        ClimateIncDec ptemp_;
        ClimateSend request_;
        ClimateHelp help_;
};

}  // namespace internal
}  // namespace R51

#endif  // _R51_CONSOLE_CLIMATE_H_
