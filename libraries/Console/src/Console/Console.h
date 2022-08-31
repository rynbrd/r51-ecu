#ifndef _R51_CONSOLE_CONSOLE_H_
#define _R51_CONSOLE_CONSOLE_H_

#include <Arduino.h>
#include <Canny.h>
#include <Caster.h>
#include <Common.h>
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
        Console(Stream* stream, const char* prefix = nullptr) :
            stream_(stream), prefix_(prefix), reader_(stream_, buffer_, 24) { reset(); }

        // Write a message to serial.
        void handle(const R51::Message& msg) override;

        // Read a text frame from serial and emit it to the bus.
        void emit(const Caster::Yield<R51::Message>& yield) override;

        // Only read messages from serial which match this filter.
        virtual bool readFilter(const R51::Message&) const { return true; }

        // Only write messages to serial which match this filter.
        virtual bool writeFilter(const R51::Message&) const { return true; }

    private:
        Stream* stream_;
        const char* prefix_;
        Reader reader_;
        char buffer_[24];
        size_t buffer_len_;
        Event event_;

        bool parseEvent();
        void reset();
        void print(const char* msg);
        void print(const char* msg, const Printable& p);
};

}  // namespace R51

#endif  // _R51_CONSOLE_CONSOLE_H_
