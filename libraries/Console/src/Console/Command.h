#ifndef _R51_CONSOLE_COMMAND_H_
#define _R51_CONSOLE_COMMAND_H_

#include <Arduino.h>
#include <Caster.h>
#include <Core.h>
#include "Console.h"

namespace R51::internal {

class Command {
    public:
        Command() {}
        virtual ~Command() = default;

        // Return the subcommand for the given arg.
        virtual Command* next(char* arg) = 0;

        // Run the command. The command may yield messages or print information
        // to the user. The arg is the string passed to the previous command to
        // choose this one.
        virtual void run(Console* console, char* arg, const Caster::Yield<Message>& yield) = 0;

        // Return true if this command consumes the remainder of the line for
        // the next arg.
        virtual bool line() { return false; }
};

}  // namespace R51::internal

#endif  // _R51_CONSOLE_COMMAND_H_
