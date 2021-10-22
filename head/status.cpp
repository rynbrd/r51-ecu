#include "status.h"

namespace ECU {

size_t printStatus(Stream* stream, Status status) {
    switch (status) {
        case OK:
            return stream->print("OK");
        case NOENT:
            return stream->print("NOENT");
        case ERROR:
            return stream->print("ERROR");
    }
    return 0;
}

}
