#ifndef _R51_CONSOLE_EVENT_H_
#define _R51_CONSOLE_EVENT_H_

#include <Arduino.h>
#include <Caster.h>
#include <Core.h>
#include "Command.h"
#include "Console.h"
#include "Error.h"

namespace R51::internal {

class EventSendRunCommand : public Command {
    public:
        Command* next(char*) override {
            return TooManyArgumentsCommand::get();
        }

        // Run the command. Parses and yields the event.
        void run(Console* console, char* arg, const Caster::Yield<Message>& yield) override;

    private:
        Event event_;
};

class EventSendCommand : public NotEnoughArgumentsCommand {
    public:
        Command* next(char*) override {
            return &run_;
        }

    private:
        EventSendRunCommand run_;
};

class EventMuteCommand : public Command {
    public:
        Command* next(char*) override {
            return TooManyArgumentsCommand::get();
        }

        void run(Console* console, char*, const Caster::Yield<Message>&) override;
};

class EventUnmuteCommand : public Command {
    public:
        Command* next(char*) override {
            return TooManyArgumentsCommand::get();
        }

        void run(Console* console, char*, const Caster::Yield<Message>&) override;
};

class EventCommand : public NotEnoughArgumentsCommand {
    public:
        Command* next(char* arg) override {
            if (strcmp(arg, "send") == 0)  {
                return &send_;
            } else if (strcmp(arg, "mute") == 0) {
                return &mute_;
            } else if (strcmp(arg, "unmute") == 0) {
                return &unmute_;
            }
            return NotFoundCommand::get();
        }

    private:
        EventSendCommand send_;
        EventMuteCommand mute_;
        EventUnmuteCommand unmute_;
};

}  // namespace R51::internal

#endif  // _R51_CONSOLE_EVENT_H_
