#ifndef _R51_CONTROLLER_J1939_H_
#define _R51_CONTROLLER_J1939_H_

#include "Config.h"

#include <Canny.h>
#include "Debug.h"

namespace R51 {

// J1939 connection which filters and buffers frames; and logs errors to serial.
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

}  // namespace R51

#endif  // _R51_CONTROLLER_J1939_H_
