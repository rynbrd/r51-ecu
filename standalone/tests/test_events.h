#ifndef __R51_TESTS_TEST_EVENTS_
#define __R51_TESTS_TEST_EVENTS_

#include <Canny.h>

#include "src/events.h"

test(MessageTest, SystemEvent) {
    SystemEvent event{0x01, {0x00, 0xFF, 0xFF, 0xFF, 0xFF}};
    Message msg(event);
    assertEqual(&event, &msg.system_event());
}

test(MessageTest, CANFrame) {
    Canny::Frame frame(0x123, 4, {0x01, 0x02, 0x03, 0x04});
    Message msg(frame);
    assertEqual(&frame, &msg.can_frame());
}

#endif  // __R51_TESTS_TEST_EVENTS_
