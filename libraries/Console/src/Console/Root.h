#ifndef _R51_CONSOLE_ROOT_H_
#define _R51_CONSOLE_ROOT_H_

#include <Arduino.h>
#include "CAN.h"
#include "Command.h"
#include "Console.h"
#include "Error.h"
#include "Event.h"
#include "J1939.h"
#include "Scratch.h"

namespace R51::internal {

// Root command.
class RootCommand : public Command {
    public:
        RootCommand(Scratch* scratch) :
                scratch_(scratch), scratch_enable_(scratch != nullptr) {}

        Command* next(char* arg) override {
            if (strcmp(arg, "can") == 0) {
                return &can_;
            } else if (strcmp(arg, "event") == 0) {
                return &event_;
            } else if (strcmp(arg, "j1939") == 0) {
                return &j1939_;
            } else if (scratch_enable_ && strcmp(arg, "scratch") == 0) {
                return &scratch_;
            }
            return NotFoundCommand::get();
        }

        // Run the command. The command may yield messages or print information
        // to the user.
        void run(Console* console, char*, const Caster::Yield<Message>&) {
            console->stream()->println("console: command incomplete");
        }

    private:
        CANCommand can_;
        EventCommand event_;
        J1939Command j1939_;
        ScratchCommand scratch_;
        bool scratch_enable_;
};

}  // namespace R51::internal

#endif  // _R51_CONSOLE_ROOT_H_
