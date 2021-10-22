#include "mcp2515.h"

#include "debug.h"
#include "status.h"

namespace ECU {

Mcp2515::Mcp2515(uint8_t cs_pin, CanSpeed can_speed) {
    speed_ = can_speed;
    client_ = new MCP_CAN(cs_pin);
}

Mcp2515::~Mcp2515() {
    delete client_;
}

Status Mcp2515::begin() {
    uint8_t mcp_speed;
    switch (speed_) {
        default:
        case CAN_SPEED_250K:
            Debug.println("set CAN speed to 250K");
            mcp_speed = CAN_250KBPS;
            break;
        case CAN_SPEED_500K:
            Debug.println("set CAN speed to 500K");
            mcp_speed = CAN_500KBPS;
            break;
    }
    //if (client_->begin(MCP_ANY, mcp_speed, MCP_16MHZ) != CAN_OK) {
    if (client_->begin(mcp_speed) != CAN_OK) {
        return ERROR;
    }
    return OK;
}

bool Mcp2515::available() const {
    return client_->checkReceive() == CAN_MSGAVAIL;
}

Status Mcp2515::read(Frame* frame) const {
    /*
    uint8_t ext;
    uint8_t mcp_err = client_->readMsgBuf(&frame->id, &ext, &frame->size, frame->data);
    if (mcp_err == OK) {
        frame->type = ext ? FRAME_EXT : FRAME_STD;
        return OK;
    } else if (mcp_err == CAN_NOMSG) {
        return NOENT;
    }
    */
    uint8_t mcp_err = client_->readMsgBufID(&frame->id, &frame->size, frame->data);
    if (mcp_err == OK) {
        frame->type = client_->isExtendedFrame() ? FRAME_EXT : FRAME_STD;
        return OK;
    } else if (mcp_err == CAN_NOMSG) {
        return NOENT;
    }
    return ERROR;
}

Status Mcp2515::write(const Frame& frame) {
    uint8_t ext;
    switch (frame.type) {
        default:
        case FRAME_STD:
            ext = CAN_STDID;
            break;
        case FRAME_EXT:
            ext = CAN_EXTID;
            break;
    }
    uint8_t mcp_err = client_->sendMsgBuf(frame.id, ext, frame.size, frame.data);
}

}
