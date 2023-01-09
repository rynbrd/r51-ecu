#ifndef _R51_CONSOLE_BCM_
#define _R51_CONSOLE_BCM_

#include <Arduino.h>
#include <Caster.h>
#include <Core.h>
#include <Vehicle.h>
#include "Command.h"
#include "Console.h"
#include "Error.h"
#include "Event.h"

namespace R51 {
namespace internal {

class BCMHelp : public Command {
    public:
        Command* next(char*) override {
            return TooManyArgumentsCommand::get();
        }

        void run(Console* console, char*, const Caster::Yield<Message>&) override {
            console->stream()->println("usage: bcm CMD");
            console->stream()->println("commands: defrost, request");
        }
};

class BCMCommand : public NotEnoughArgumentsCommand {
    public:
        BCMCommand() :
            defrost_(Event(SubSystem::BCM, (uint8_t)BCMEvent::TOGGLE_DEFROST_CMD)),
            request_(Event(SubSystem::CONTROLLER, (uint8_t)ControllerEvent::REQUEST_CMD,
                           {(uint8_t)SubSystem::BCM})) {}

        Command* next(char* arg) override {
            if (strcmp(arg, "defrost") == 0 || strcmp(arg, "d") == 0) {
                return &defrost_;
            } else if (strcmp(arg, "request") == 0 || strcmp(arg, "req") == 0) {
                return &request_;
            } else if (strcmp(arg, "help") == 0 || strcmp(arg, "h") == 0) {
                return &help_;
            }
            return NotFoundCommand::get();
        }

    private:
        EventSendCommand defrost_;
        EventSendCommand request_;
        BCMHelp help_;
};

}  // namespace internal
}  // namespace R51

#endif  // _R51_CONSOLE_BCM_
