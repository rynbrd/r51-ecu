#ifndef _R51_CONSOLE_CONSOLE_H_
#define _R51_CONSOLE_CONSOLE_H_

#include <Arduino.h>
#include <Canny.h>

namespace R51 {

class Console {
    public:
        Console(Stream* stream) : stream_(stream), j1939_mute_(false) {}
        Stream* stream() { return stream_; }
        Canny::FrameIDFilter* can_filter() { return &can_filter_; }
        bool j1939_mute() { return j1939_mute_; }
        void j1939_mute(bool mute) { j1939_mute_ = mute; }

    private:
        Stream* stream_;
        Canny::FrameIDFilter can_filter_;
        bool j1939_mute_;
};

}  // namespace R51::internal

#endif  // _R51_CONSOLE_CONSOLE_H_
