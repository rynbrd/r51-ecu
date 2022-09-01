#ifndef _R51_CONSOLE_CAN_H_
#define _R51_CONSOLE_CAN_H_

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include "Command.h"
#include "Error.h"

namespace R51::internal {

class CANSendRunCommand : public Command {
    public:
        void set(char* encoded) { buffer_ = encoded; }

        Command* next(char*) override {
            return TooManyArgumentsCommand::get();
        }

        // Run the command. Parses and yields the event.
        void run(Stream* console, const Caster::Yield<Message>& yield) override;

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

class CANCommand : public NotEnoughArgumentsCommand {
    public:
        Command* next(char* arg) {
            if (strcmp(arg, "send") == 0) {
                return &send_;
            }
            return NotFoundCommand::get();
        }

    private:
        CANSendCommand send_;
};

}

#endif  // _R51_CONSOLE_CAN_H_
