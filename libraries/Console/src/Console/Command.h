#ifndef _R51_CONSOLE_COMMAND_H_
#define _R51_CONSOLE_COMMAND_H_

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>
#include "Console.h"

namespace R51::internal {

class Command {
    public:
        Command() {}
        virtual ~Command() = default;

        // Return the subcommand for the given arg.
        virtual Command* next(char* arg) = 0;

        // Run the command. The command may yield messages or print information
        // to the user.
        virtual void run(Console* console, const Caster::Yield<Message>& yield) = 0;
};

}  // namespace R51::internal

#endif  // _R51_CONSOLE_COMMAND_H_
