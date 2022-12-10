#ifndef _R51_CONSOLE_CAN_H_
#define _R51_CONSOLE_CAN_H_

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include "Command.h"
#include "Console.h"
#include "Error.h"

namespace R51::internal {

class CANSendRunCommand : public Command {
    public:
        Command* next(char*) override {
            return TooManyArgumentsCommand::get();
        }

        // Run the command. Parses and yields the event.
        void run(Console* console, char* arg, const Caster::Yield<Message>& yield) override;

    private:
        Canny::CAN20Frame frame_;
};

class CANSendCommand : public NotEnoughArgumentsCommand {
    public:
        Command* next(char*) override {
            return &run_;
        }

    private:
        CANSendRunCommand run_;
};

class CANFilterAllowRunCommand : public Command {
    public:
        Command* next(char*) override {
            return TooManyArgumentsCommand::get();
        }

        void run(Console* console, char* arg, const Caster::Yield<Message>&) override;
};

class CANFilterAllowCommand : public NotEnoughArgumentsCommand {
    public:
        Command* next(char*) override {
            return &run_;
        }

    private:
        CANFilterAllowRunCommand run_;
};

class CANFilterDropRunCommand : public Command {
    public:
        Command* next(char*) override {
            return TooManyArgumentsCommand::get();
        }

        void run(Console* console, char* arg, const Caster::Yield<Message>&) override;
};

class CANFilterDropCommand : public NotEnoughArgumentsCommand {
    public:
        Command* next(char*) override {
            return &run_;
        }

    private:
        CANFilterDropRunCommand run_;
};

class CANFilterCommand : public NotEnoughArgumentsCommand {
    public:
        Command* next(char* arg) override {
            if (strcmp(arg, "allow") == 0) {
               return &allow_; 
            } else if (strcmp(arg, "drop") == 0) {
                return &drop_;
            }
            return NotFoundCommand::get();
        }

    private:
        CANFilterAllowCommand allow_;
        CANFilterDropCommand drop_;
};

class CANCommand : public NotEnoughArgumentsCommand {
    public:
        Command* next(char* arg) override {
            if (strcmp(arg, "send") == 0) {
                return &send_;
            } else if (strcmp(arg, "filter") == 0) {
                return &filter_;
            }
            return NotFoundCommand::get();
        }

    private:
        CANSendCommand send_;
        CANFilterCommand filter_;
};

}  // namespace R51::internal

#endif  // _R51_CONSOLE_CAN_H_
