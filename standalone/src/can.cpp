#include "can.h"

#include "debug.h"
#include "same51_can.h"


void Same51Can::begin() {
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

void Same51Can::receive(const Broadcast& broadcast) {
    if (!init_) {
        return;
    }

    uint8_t err = client_.readMsgBuf(&frame_.id, &frame_.len, frame_.data);
    if (err == CAN_OK) {
        broadcast(frame_);
        return;
    }
    if (err != CAN_NOMSG) {
        ERROR_MSG_VAL("can: read failed: error code ", err);
    }
}

void Same51Can::send(const Frame& frame) {
    if (!init_) {
        return;
    }

    uint8_t err;
    uint8_t attempts = 0;
    do {
        err = client_.sendMsgBuf(frame.id, 0, frame.len, (uint8_t*)frame.data);
        attempts++;
    } while (err != CAN_OK && attempts <= retries_);

    if (err != CAN_OK) {
        ERROR_MSG_VAL("can: write failed: error code ", err);
        ERROR_MSG_FRAME("can: dropped frame ", frame);
    }
}
