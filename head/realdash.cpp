#include "realdash.h"

static const byte frame44Prefix[4] = {0x44, 0x33, 0x22, 0x11};

bool parseBuffer(byte* buf, uint32_t* id, byte* data) {
}

bool RealDash::read(uint32_t* id, uint8_t* len, byte* data) {
    // Read serial data into the incoming buffer.
    while (incoming_size_ < 17 && stream_->available() > 0) {
        incoming_buffer_[incoming_size_++] = stream_->read();
        if (incoming_buffer_[0] != frame44Prefix[0]) {
            incoming_size_ = 0;
        }
    }

    // Return if we have not received enough data.
    if (incoming_size_ < 17) {
        return false;
    }

    // Verify frame prefix.
    for (int i = 0; i < 4; i++ ) {
        if (incoming_buffer_[i] != frame44Prefix[i]) {
            return false;
        }
    }

    // Verify frame checksum.
    byte checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += incoming_buffer_[i];
    }
    if (checksum != incoming_buffer_[16]) {
        return false;
    }

    // Copy data to args.
    memcpy(id, incoming_buffer_ + 4, 4);
    memcpy(data, incoming_buffer_ + 8, 8);
    *len = 8;

    // Reset input buffer.
    incoming_size_ = 0;

    return true;
}

void RealDash::write(uint32_t id, uint8_t len, byte* data) {
    if (len > 8) {
        len = 8;
    }
    stream_->write(frame44Prefix, 4);
    stream_->write((const byte*)&id, 4);
    stream_->write(data, len);
    for (int i = 0; i < 8-len; i++) {
        stream_->write((byte)0);
    }
}
