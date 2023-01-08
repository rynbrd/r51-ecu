#ifndef _R51_CONSOLE_CONSOLE_H_
#define _R51_CONSOLE_CONSOLE_H_

#include <Arduino.h>
#include <Canny.h>

namespace R51 {

class Console {
    public:
        Console(Stream* stream) : stream_(stream), event_mute_(true), j1939_mute_(true) {
            can_filter_.mode(Canny::FilterMode::DROP);
        }
        Stream* stream() { return stream_; }
        Canny::FrameIDFilter* can_filter() { return &can_filter_; }
        bool event_mute() { return event_mute_; }
        void event_mute(bool mute) { event_mute_ = mute; }
        bool j1939_mute() { return j1939_mute_; }
        void j1939_mute(bool mute) { j1939_mute_ = mute; }

    private:
        Stream* stream_;
        Canny::FrameIDFilter can_filter_;
        bool event_mute_;
        bool j1939_mute_;
};

}  // namespace R51::internal

#endif  // _R51_CONSOLE_CONSOLE_H_
