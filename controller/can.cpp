#include "can.h"

#include "debug.h"
#include "same51_can.h"

void CanConnection::begin() {
    while (!init_) {
        uint8_t err = client_.begin(MCP_ANY, baudrate_, MCAN_MODE_CAN);
        if (err == CAN_OK) {
            init_ = true;
        } else {
            ERROR_MSG_VAL("can: connect failed: error code ", err);
            delay(1000);
        }
    }
}

bool CanConnection::read(uint32_t* id, uint8_t* len, byte* data) {
    if (!init_) {
        return false;
    }
    uint8_t err = client_.readMsgBuf(id, len, data);
    if (err == CAN_OK) {
        return true;
    }
    if (err != CAN_NOMSG) {
        ERROR_MSG_VAL("can: read failed: error code ", err);
    }
    return false;
}

bool CanConnection::write(uint32_t id, uint8_t len, byte* data) {
    if (!init_) {
        return false;
    }
    uint8_t err;
    uint8_t attempts = 0;
    do {
        err = client_.sendMsgBuf(id, 0, len, data);
        attempts++;
    } while (err != CAN_OK && attempts <= retries_);

    if (err != CAN_OK) {
        ERROR_MSG_VAL("can: write failed: error code ", err);
        ERROR_MSG_FRAME("can: dropped frame ", id, len, data);
        return false;
    }
    return true;
}
