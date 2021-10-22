#include "can.h"

#include <Stream.h>

namespace ECU {

size_t printFrame(Stream* stream, const Frame& frame) {
    size_t n = 0;
    n += stream->print(frame.id, HEX);
    n += stream->print("#");
    for (int i = 0; i < frame.size; i++) {
        if (frame.data[i] <= 0x0F) {
            n += stream->print("0");
        }
        n += stream->print(frame.data[i], HEX);
        if (i < frame.size-1) {
            n += stream->print(":");
        }
    }
    return n;
}

}
