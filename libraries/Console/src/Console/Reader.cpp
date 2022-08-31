#include "Reader.h"

namespace R51 {

Reader::Error Reader::read(size_t* len, bool line) {
    byte b;
    while (stream_->available()) {
        b = stream_->peek();
        if (b == '\r' || b == '\n') {
            stream_->read();
            if (len_ == 0) {
                // Discard extra newlines.
                continue;
            }
            // Return the last token on the line without trailing whitespace.
            if (line) {
                buffer_[len_ - space_] = 0;
            } else {
                buffer_[len_] = 0;
            }
            if (len != nullptr) {
                *len = len_;
            }
            len_ = 0;
            space_ = 0;
            return EOL;
        } else if (b == ' ' || b == '\t') {
            stream_->read();
            // Keep track of trailing space.
            ++space_;
            if (line && len_ > 0) {
                buffer_[len_++] = b;
            }
        } else {
            // Emit a word if we've previously encountered space.
            if (!line && len_ > 0 && space_ > 0) {
                buffer_[len_] = 0;
                if (len != nullptr) {
                    *len = len_;
                }
                len_ = 0;
                space_ = 0;
                return EOW;
            }
            // Append data to the buffer and consume the byte.
            stream_->read();
            buffer_[len_++] = b;
            space_ = 0;
        }
    }
    return FIFO;
}

Reader::Error Reader::word(size_t* len) {
    return read(len, false);
}

Reader::Error Reader::line(size_t* len) {
    return read(len, true);
}

void Reader::reset(char* buffer, size_t size) {
    if (buffer != nullptr) {
        buffer_ = buffer;
        size_ = size;
    }
    len_ = 0;
    space_ = 0;
}

}  // namespace R51
