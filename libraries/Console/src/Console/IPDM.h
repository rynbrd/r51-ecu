#ifndef _R51_CONSOLE_IPDM_
#define _R51_CONSOLE_IPDM_

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

class IPDMHelp : public Command {
    public:
        Command* next(char*) override {
            return TooManyArgumentsCommand::get();
        }

        void run(Console* console, char*, const Caster::Yield<Message>&) override {
            console->stream()->println("usage: ipdm CMD");
            console->stream()->println("commands: request");
        }
};

class IPDMCommand : public NotEnoughArgumentsCommand {
    public:
        IPDMCommand() :
            request_(Event(SubSystem::CONTROLLER, (uint8_t)ControllerEvent::REQUEST_CMD,
                           {(uint8_t)SubSystem::IPDM})) {}

        Command* next(char* arg) override {
            if (strcmp(arg, "request") == 0 || strcmp(arg, "req") == 0) {
                return &request_;
            } else if (strcmp(arg, "help") == 0 || strcmp(arg, "h") == 0) {
                return &help_;
            }
            return NotFoundCommand::get();
        }

    private:
        EventSendCommand request_;
        IPDMHelp help_;
};

}  // namespace internal
}  // namespace R51

#endif  // _R51_CONSOLE_IPDM_
