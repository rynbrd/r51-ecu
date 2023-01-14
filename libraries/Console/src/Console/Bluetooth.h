#ifndef _R51_CONSOLE_BLUETOOTH_H_
#define _R51_CONSOLE_BLUETOOTH_H_

#include <Arduino.h>
#include <Bluetooth.h>
#include <Core.h>
#include "Command.h"
#include "Console.h"
#include "Error.h"
#include "Event.h"

namespace R51 {
namespace internal {

class BluetoothHelp : public Command {
    public:
        Command* next(char*) override {
            return TooManyArgumentsCommand::get();
        }

        void run(Console* console, char*, const Caster::Yield<Message>&) override {
            console->stream()->println("usage: bluetooth CMD");
            console->stream()->println("commands: disconnect, forget");
        }
};

class BluetoothCommand : public NotEnoughArgumentsCommand {
    public:
        BluetoothCommand() :
            disconnect_(Event(SubSystem::BLUETOOTH, (uint8_t)BluetoothEvent::DISCONNECT_CMD)),
            forget_(Event(SubSystem::BLUETOOTH, (uint8_t)BluetoothEvent::FORGET_CMD)),
            request_(Event(SubSystem::CONTROLLER, (uint8_t)ControllerEvent::REQUEST_CMD,
                           {(uint8_t)SubSystem::BLUETOOTH})) {}

        Command* next(char* arg) override {
            if (strcmp(arg, "disconnect") == 0 || strcmp(arg, "d") == 0) {
                return &disconnect_;
            } else if (strcmp(arg, "forget") == 0 || strcmp(arg, "f") == 0) {
                return &forget_;
            } else if (strcmp(arg, "request") == 0 || strcmp(arg, "req") == 0) {
                return &request_;
            } else if (strcmp(arg, "help") == 0 || strcmp(arg, "h") == 0) {
                return &help_;
            }
            return NotFoundCommand::get();
        }

    private:
        EventSendCommand disconnect_;
        EventSendCommand forget_;
        EventSendCommand request_;
        BluetoothHelp help_;
};

}  // namespace internal
}  // namespace R51

#endif  // _R51_CONSOLE_BLUETOOTH_H_
