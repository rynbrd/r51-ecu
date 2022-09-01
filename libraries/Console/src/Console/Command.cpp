#include "Command.h"

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>
#include "Event.h"
#include "Error.h"

namespace R51::internal {

// Root command.
class RootCommand : public Command {
    public:
        Command* next(char* arg) override {
            if (strcmp(arg, "event") == 0) {
                return &event_;
            }
            return NotFoundCommand::get();
        }

        // Run the command. The command may yield messages or print information
        // to the user.
        void run(Stream* console, const Caster::Yield<Message>&) {
            console->println("command incomplete");
        }

    private:
        EventCommand event_;
};

Command* Command::root() {
    static RootCommand c;
    return &c;
}

}  // namespace R51::internal
