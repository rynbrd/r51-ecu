#ifndef _R51_CONSOLE_SCRATCH_H_
#define _R51_CONSOLE_SCRATCH_H_

#include <Arduino.h>
#include <Core.h>
#include "Command.h"
#include "Error.h"

namespace R51::internal {

class ScratchSetCommand : public Command {
    public:
        ScratchSetCommand(Scratch* scratch) : scratch_(scratch) {}

        void run(Console* console, char* arg, const Caster::Yield<Message>&) override;

        Command* next(char*) override {
            return TooManyArgumentsCommand::get();
        }

    private:
        Scratch* scratch_;
};

class ScratchCommand : public NotEnoughArgumentsCommand {
    public:
        ScratchCommand(Scratch* scratch) : set_(scratch) {}

        Command* next(char*) override {
            return &set_;
        }

        bool line() override { return true; }

    private:
        ScratchSetCommand set_;
};

}

#endif  // _R51_CONSOLE_SCRATCH_H_
