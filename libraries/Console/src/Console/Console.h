#ifndef _R51_CONSOLE_CONSOLE_H_
#define _R51_CONSOLE_CONSOLE_H_

#include <Arduino.h>
#include <Canny.h>

namespace R51 {

class Console {
    public:
        Console(Stream* stream) : stream_(stream) {}
        Stream* stream() { return stream_; }
        Canny::FrameIDFilter* can_filter() { return &can_filter_; }

    private:
        Stream* stream_;
        Canny::FrameIDFilter can_filter_;
};

}  // namespace R51::internal

#endif  // _R51_CONSOLE_CONSOLE_H_
