#ifndef __R51_TESTS_TESTING__
#define __R51_TESTS_TESTING__

#include <Canny.h>

#include "mock_broadcast.h"


bool checkFrameCount(const MockBroadcast& cast, uint8_t expect) {
    if (cast.count() == expect) {
        return true;
    }
    Serial.print("frame counts not equal:\n  actual: ");
    Serial.print(cast.count());
    Serial.print("\n  expect: ");
    Serial.println(expect);
    return false;
}

bool checkFrameEquals(const Canny::Frame& left, const Canny::Frame& right) {
    if (left != right) {
        Serial.println("frames not equal:");
        Serial.print("  actual: ");
        Serial.println(left);
        Serial.print("  expect: ");
        Serial.println(right);
        return false;
    }
    return true;
}

#endif  // __R51_TESTS_TESTING__
