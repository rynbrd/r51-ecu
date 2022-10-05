#ifndef _R51_BRIDGE_J1939_H_
#define _R51_BRIDGE_J1939_H_

#include "Config.h"

#include <Canny.h>
#include <Caster.h>
#include <Common.h>
#include "Debug.h"

namespace R51 {

// J1939 connection which that logs errors to serial.
class J1939Connection : public Canny::BufferedConnection {
    public:
        J1939Connection(Canny::Connection* can) :
                Canny::BufferedConnection(can, J1939_READ_BUFFER, J1939_WRITE_BUFFER, 8) {}

        // Log read errors to debug serial.
        void onReadError(Canny::Error err) const override {
            DEBUG_MSG_VAL("j1939: read error: ", err);
        }

        // Log write errors to debug serial.
        void onWriteError(Canny::Error err, const Canny::Frame& frame) const override {
            DEBUG_MSG_VAL("j1939: write error: ", err);
            DEBUG_MSG_VAL("j1939: dropped frame: ", frame);
        }
};


// Broadcasts Events over J1939.
class J1939Events : public Caster::Node<Message> {
    public:
        J1939Events();

        // Translate Events to and from J1939 messages.
        void handle(const Message& msg, const Caster::Yield<Message>& yield) override;

        // Does nothing.
        void emit(const Caster::Yield<Message>&) override {}
    private:
        void handleEvent(const Event& event,
                const Caster::Yield<Message>& yield);
        void handleJ1939Message(const Canny::J1939Message& msg,
                const Caster::Yield<Message>& yield);

        Canny::J1939Message j1939_;
        Event event_;
};

}  // namespace R51

#endif  // _R51_BRIDGE_J1939_H_
