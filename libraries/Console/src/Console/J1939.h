#ifndef _R51_CONSOLE_J1939_H_
#define _R51_CONSOLE_J1939_H_

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include "CAN.h"
#include "Command.h"
#include "Console.h"
#include "Error.h"

namespace R51::internal {

class J1939SendRunCommand : public Command {
    public:
        Command* next(char*) override {
            return TooManyArgumentsCommand::get();
        }

        // Run the command. Parses and yields the message.
        void run(Console* console, char* arg, const Caster::Yield<Message>& yield) override;

    private:
        Canny::J1939Message msg_;
};

class J1939SendCommand : public NotEnoughArgumentsCommand {
    public:
        Command* next(char*) override {
            return &run_;
        }

    private:
        J1939SendRunCommand run_;
};

class J1939Command : public NotEnoughArgumentsCommand {
    public:
        Command* next(char* arg) override {
            if (strcmp(arg, "send") == 0) {
                return &send_;
            }
            return NotFoundCommand::get();
        }

    private:
        J1939SendCommand send_;
};

}  // namespace R51::internal

#endif  // _R51_CONSOLE_J1939_H_
