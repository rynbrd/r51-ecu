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
        void set(char* encoded) { buffer_ = encoded; }

        Command* next(char*) override {
            return TooManyArgumentsCommand::get();
        }

        // Run the command. Parses and yields the event.
        void run(Console* console, const Caster::Yield<Message>& yield) override;

    private:
        char* buffer_;
        Canny::Frame frame_;
};

class CANSendCommand : public NotEnoughArgumentsCommand {
    public:
        Command* next(char* arg) override {
            run_.set(arg);
            return &run_;
        }

    private:
        CANSendRunCommand run_;
};

class CANFilterAllowRunCommand : public Command {
    public:
        void set(char* arg) { arg_ = arg; }

        Command* next(char*) override {
            return TooManyArgumentsCommand::get();
        }

        void run(Console* console, const Caster::Yield<Message>&) override {
            if (strcmp(arg_, "all") == 0) {
                console->can_filter()->mode(Canny::FilterMode::ALLOW);
            } else {
                uint32_t id = strtoul(arg_, nullptr, 16);
                if (id == 0) {
                    console->stream()->println("invalid frame id");
                } else {
                    console->can_filter()->allow(id);
                }
            }
        }

    private:
        char* arg_;
};

class CANFilterAllowCommand : public NotEnoughArgumentsCommand {
    public:
        Command* next(char* arg) override {
            run_.set(arg);
            return &run_;
        }

    private:
        CANFilterAllowRunCommand run_;
};

class CANFilterDropRunCommand : public Command {
    public:
        void set(char* arg) { arg_ = arg; }

        Command* next(char*) override {
            return TooManyArgumentsCommand::get();
        }

        void run(Console* console, const Caster::Yield<Message>&) override {
            if (strcmp(arg_, "all") == 0) {
                console->can_filter()->mode(Canny::FilterMode::DROP);
            } else {
                uint32_t id = strtoul(arg_, nullptr, 16);
                if (id == 0) {
                    console->stream()->println("invalid frame id");
                } else {
                    console->can_filter()->drop(id);
                }
            }
        }

    private:
        char* arg_;
};

class CANFilterDropCommand : public NotEnoughArgumentsCommand {
    public:
        Command* next(char* arg) override {
            run_.set(arg);
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
        Command* next(char* arg) {
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

}

#endif  // _R51_CONSOLE_CAN_H_
