#ifndef _R51_CONSOLE_COMMAND_H_
#define _R51_CONSOLE_COMMAND_H_

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>

namespace R51::internal {

class RootCommand;

class Command {
    public:
        Command() {}
        virtual ~Command() = default;

        // Return the subcommand for the given arg.
        virtual Command* next(char* arg) = 0;

        // Run the command. The command may yield messages or print information
        // to the user.
        virtual void run(Stream* console, const Caster::Yield<Message>& yield) = 0;

        // Get the root command instance.
        static Command* root();
};

}  // namespace R51::internal

#endif  // _R51_CONSOLE_COMMAND_H_
