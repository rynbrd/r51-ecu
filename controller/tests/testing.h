#ifndef __R51_TESTS_TESTING__
#define __R51_TESTS_TESTING__

#include "src/bus.h"


void printFrame(const Frame& frame) {
    Serial.print(frame.id, HEX);
    Serial.print("#");
    for (int i = 0; i < frame.len; i++) {
        if (frame.data[i] <= 0x0F) {
            Serial.print("0");
        }
        Serial.print(frame.data[i], HEX);
        if (i < frame.len-1) {
            Serial.print(":");
        }
    }
}

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

bool checkFrameEquals(const Frame& left, const Frame& right) {
    if (!frameEquals(left, right)) {
        Serial.print("frames not equal:\n  actual: ");
        printFrame(left);
        Serial.print("\n  expect: ");
        printFrame(right);
        Serial.println();
        return false;
    }
    return true;
}

#endif  // __R51_TESTS_TESTING__
