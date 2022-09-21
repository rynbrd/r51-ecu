#ifndef _R51_CONSOLE_ERROR_H_
#define _R51_CONSOLE_ERROR_H_

#include <Arduino.h>
#include "Command.h"
#include "Console.h"

namespace R51::internal {

// Base error class.
class ErrorCommand : public Command {
    public:
        Command* next(char*) override {
            return this;
        }
};

// Prints a "command not found" error when run.
class NotFoundCommand : public ErrorCommand {
    public:
        void run(Console* console, char*, const Caster::Yield<Message>&) {
            console->stream()->println("console: command not found");
        }

        static Command* get() {
            static NotFoundCommand c;
            return &c;
        }
};

// Prints a "not enough arguments" error when run.
class NotEnoughArgumentsCommand : public ErrorCommand {
    public:
        void run(Console* console, char*, const Caster::Yield<Message>&) {
            console->stream()->println("console: not enough arguments");
        }

        static Command* get() {
            static NotEnoughArgumentsCommand c;
            return &c;
        }
};

// Prints a "too many arguments" error when run.
class TooManyArgumentsCommand : public ErrorCommand {
    public:
        void run(Console* console, char*, const Caster::Yield<Message>&) {
            console->stream()->println("console: too many arguments");
        }

        static Command* get() {
            static TooManyArgumentsCommand c;
            return &c;
        }
};

}  // namespace R51::internal

#endif  // _R51_CONSOLE_ERROR_H_
