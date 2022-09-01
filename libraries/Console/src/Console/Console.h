#ifndef _R51_CONSOLE_CONSOLE_H_
#define _R51_CONSOLE_CONSOLE_H_

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include <Common.h>
#include "Command.h"
#include "Reader.h"

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
class Console : public Caster::Node<R51::Message> {
    public:
        Console(Stream* stream) :
            stream_(stream),
            reader_(stream_, buffer_, 32),
            root_(internal::Command::root()),
            command_(root_) {}

        // Write a message to serial.
        void handle(const R51::Message& msg) override;

        // Read a text frame from serial and emit it to the bus.
        void emit(const Caster::Yield<R51::Message>& yield) override;

        // Only read messages from serial which match this filter.
        virtual bool readFilter(const R51::Message&) const { return true; }

        // Only write messages to serial which match this filter.
        virtual bool writeFilter(const R51::Message&) const { return true; }

    private:
        char buffer_[32];
        Stream* stream_;
        Reader reader_;
        internal::Command* root_;
        internal::Command* command_;
};

}  // namespace R51

#endif  // _R51_CONSOLE_CONSOLE_H_
