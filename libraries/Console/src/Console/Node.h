#ifndef _R51_CONSOLE_NODE_H_
#define _R51_CONSOLE_NODE_H_

#include <Arduino.h>
#include <Caster.h>
#include <Common.h>
#include "Command.h"
#include "Console.h"
#include "Reader.h"
#include "Root.h"

namespace R51 {

// A node which writes messages to a stream in human readable format. Also
// supports reading events from the stream.
//
// Events have the following syntax:
//     SS:ID#FF:FF:FF:FF:FF
//
//   SS - The subsystem value in hex.
//   ID - The ID value in hex.
//   FF:FF:FF:FF:FF - The six data bytes of the event. This may be zero or more
//     bytes. The remainder is automatically badded with 0xFF bytes.
//
// Each message printed is terminated with a CR+LF.
class ConsoleNode : public Caster::Node<R51::Message> {
    public:
        ConsoleNode(Stream* stream) :
            console_(stream),
            reader_(stream, buffer_, 256),
            command_(&root_) {}

        // Write a message to serial.
        void handle(const R51::Message& msg, const Caster::Yield<R51::Message>&) override;

        // Read a text frame from serial and emit it to the bus.
        void emit(const Caster::Yield<R51::Message>& yield) override;

        Console* console() { return &console_; }

    private:
        char buffer_[256];
        Console console_;
        Reader reader_;
        internal::RootCommand root_;
        internal::Command* command_;
};

}  // namespace R51

#endif  // _R51_CONSOLE_NODE_H_
