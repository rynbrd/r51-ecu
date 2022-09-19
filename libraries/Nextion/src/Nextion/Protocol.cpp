#include "Protocol.h"
#include <Arduino.h>

namespace Nextion {

void Protocol::send(const uint8_t* command, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        stream_->write(command[i]);
    }
    stream_->write(0xFF);
    stream_->write(0xFF);
    stream_->write(0xFF);
}

void Protocol::send(const char* command) {
    for (size_t i = 0; command[i] != 0; ++i) {
        stream_->write(command[i]);
    }
    stream_->write(0xFF);
    stream_->write(0xFF);
    stream_->write(0xFF);
}

size_t Protocol::recv(uint8_t* message, size_t size) {
    if (!stream_->available()) {
        return false;
    }

    int b;
    int n = 0;
    size_t terminate = 0;

    while (terminate < 3) {
        b = stream_->read();
        if (b == -1) {
            continue;
        }

        if (n >= (int)size) {
            n = -1;
        } else if (n >= 0) {
            message[n++] = (uint8_t)b;
        }
        if (b == 0xFF) {
            ++terminate;
        } else {
            terminate = 0;
        }
    }
    if (n == -1) {
        return 0;
    }
    return n - 3;
}

}  // namespace Nextion
