#include "can.h"

#include "debug.h"
#include "mcp_can.h"

CanConnection::~CanConnection() {
    if (mcp_ != nullptr) {
        delete mcp_;
        mcp_ = nullptr;
    }
}

bool CanConnection::begin(uint8_t cs_pin, uint8_t baudrate, uint8_t clockset) {
    mcp_ = new MCP_CAN(cs_pin);
    uint8_t err = mcp_->begin(MCP_ANY, baudrate, clockset);
    if (err != CAN_OK) {
        ERROR_MSG_VAL("can: connect failed: error code ", err);
        return false;
    }
    err = mcp_->setMode(MCP_NORMAL);
    if (err != CAN_OK) {
        ERROR_MSG_VAL("can: set normal mode failed: error code ", err);
        return false;
    }
    return true;
}

bool CanConnection::read(uint32_t* id, uint8_t* len, byte* data) {
    if (mcp_ == nullptr) {
        return false;
    }
    uint8_t err = mcp_->readMsgBuf(id, len, data);
    if (err != CAN_OK) {
        if (err != CAN_NOMSG) {
            ERROR_MSG_VAL("can: read failed: error code ", err);
        }
        return false;
    }
    return true;
}

bool CanConnection::write(uint32_t id, uint8_t len, byte* data) {
    if (mcp_ == nullptr) {
        return false;
    }
    uint8_t err;
    uint8_t attempts = 0;
    do {
        err = mcp_->sendMsgBuf(id, 0, len, data);
        attempts++;
    } while (err != CAN_OK && attempts <= retries_);

    if (err != CAN_OK) {
        ERROR_MSG_VAL("can: write failed: error code ", err);
        ERROR_MSG_FRAME("can: dropped frame ", id, len, data);
        return false;
    }
    return true;
}
