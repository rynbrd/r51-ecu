#ifndef _R51_CONSOLE_ROOT_H_
#define _R51_CONSOLE_ROOT_H_

#include <Arduino.h>
#include "BCM.h"
#include "CAN.h"
#include "Climate.h"
#include "Command.h"
#include "Console.h"
#include "Error.h"
#include "Event.h"
#include "IPDM.h"
#include "J1939.h"
#include "Scratch.h"

namespace R51::internal {

// Root command.
class RootCommand : public Command {
    public:
        RootCommand() {}

        Command* next(char* arg) override {
            if (strcmp(arg, "can") == 0 || strcmp(arg, "f") == 0) {
                return &can_;
            } else if (strcmp(arg, "event") == 0 || strcmp(arg, "e") == 0) {
                return &event_;
            } else if (strcmp(arg, "j1939") == 0 || strcmp(arg, "j") == 0) {
                return &j1939_;
            } else if (strcmp(arg, "climate") == 0 || strcmp(arg, "c") == 0) {
                return &climate_;
            } else if (strcmp(arg, "bcm") == 0 || strcmp(arg, "b") == 0) {
                return &bcm_;
            } else if (strcmp(arg, "ipdm") == 0 || strcmp(arg, "i") == 0) {
                return &ipdm_;
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
        ClimateCommand climate_;
        BCMCommand bcm_;
        IPDMCommand ipdm_;
};

}  // namespace R51::internal

#endif  // _R51_CONSOLE_ROOT_H_
