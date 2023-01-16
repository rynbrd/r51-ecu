#ifndef _R51_STANDALONE_J1939_H_
#define _R51_STANDALONE_J1939_H_

#include "Config.h"

#include <Canny.h>
#include <Canny/MCP2518.h>
#include <Core.h>
#include "Debug.h"

Canny::MCP2518<Canny::J1939Message> J1939(MCP2518_CS_PIN);

namespace R51 {

// J1939 connection which that logs errors to serial.
class J1939Connection : public Canny::BufferedConnection<Canny::J1939Message> {
    public:
        J1939Connection() :
            Canny::BufferedConnection<Canny::J1939Message>(
                    &J1939, J1939_READ_BUFFER, J1939_WRITE_BUFFER) {}

        bool begin() {
            // Initialize controller.
            return J1939.begin(J1939_CAN_MODE);
        }

        // Log read errors to debug serial.
        void onReadError(Canny::Error err) const override {
            DEBUG_MSG_VAL("j1939: read error: ", err);
        }

        // Log write errors to debug serial.
        void onWriteError(Canny::Error err, const Canny::J1939Message& msg) const override {
            DEBUG_MSG_VAL("j1939: write error: ", err);
            DEBUG_MSG_OBJ("j1939: dropped frame: ", msg);
        }
};

}  // namespace R51

#endif  // _R51_STANDALONE_J1939_H_
