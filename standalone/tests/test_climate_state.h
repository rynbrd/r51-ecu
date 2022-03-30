#ifndef __R51_TESTS_TEST_CLIMATE_STATE__
#define __R51_TESTS_TEST_CLIMATE_STATE__

#include <Arduino.h>
#include <AFake.h>
#include <AUnit.h>

#include "mock_broadcast.h"
#include "src/climate.h"
#include "testing.h"

using namespace aunit;


#define INIT_STATE() \
    AFake::FakeClock clock;\
    AFake::FakeGPIO gpio;\
    Climate climate(&clock, &gpio);

bool checkStateFrames(Climate* climate, const Frame& state54A, const Frame& state54B, const Frame& expect) {
    MockBroadcast cast(1, 0x5400);
    climate->send(state54A);
    climate->send(state54B);
    climate->receive(cast.impl);
    return checkFrameCount(cast, 1) &&
        checkFrameEquals(cast.frames()[0], expect);
}

test(ClimateStateTest, OffAndHeartbeat) {
    INIT_STATE();
    MockBroadcast cast(1, 0x5400);

    Frame state54A = {0x54A, 8, {0x3C, 0x3E, 0x7F, 0x80, 0x00, 0x00, 0x00, 0x2C}};
    Frame state54B = {0x54B, 8, {0xF2, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x02}};
    Frame expect = {CLIMATE_STATE_FRAME_ID, 8, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2C}};

    climate.receive(cast.impl);
    assertTrue(checkFrameCount(cast, 0));

    cast.reset();
    clock.delay(10);
    assertTrue(checkStateFrames(&climate, state54A, state54B, expect));

    clock.delay(CLIMATE_STATE_FRAME_HB);
    climate.receive(cast.impl);
    assertTrue(checkFrameCount(cast, 1) &&
        checkFrameEquals(cast.frames()[0], expect));
}

test(ClimateStateTest, Auto) {
    INIT_STATE();
    Frame state54A = {0x54A, 8, {0x3C, 0x3E, 0x7F, 0x80, 0x49, 0x49, 0x00, 0x2C}};
    Frame state54B = {0x54B, 8, {0x59, 0x8C, 0x05, 0x24, 0x00, 0x00, 0x00, 0x02}};
    Frame expect = {CLIMATE_STATE_FRAME_ID, 8, {0x27, 0x03, 0x49, 0x49, 0x00, 0x00, 0x00, 0x2C}};
    assertTrue(checkStateFrames(&climate, state54A, state54B, expect));
}

test(ClimateStateTest, AutoDual) {
    INIT_STATE();
    Frame state54A = {0x54A, 8, {0x3C, 0x3E, 0x7F, 0x80, 0x49, 0x4A, 0x00, 0x2C}};
    Frame state54B = {0x54B, 8, {0x59, 0x08, 0x02, 0xE2, 0x00, 0x00, 0x00, 0x02}};
    Frame expect = {CLIMATE_STATE_FRAME_ID, 8, {0x3F, 0x01, 0x49, 0x4A, 0x00, 0x00, 0x00, 0x2C}};
    assertTrue(checkStateFrames(&climate, state54A, state54B, expect));
}

test(ClimateStateTest, ManualRecirculate) {
    INIT_STATE();
    Frame state54A = {0x54A, 8, {0x3C, 0x3E, 0x7F, 0x80, 0x4E, 0x50, 0x00, 0x20}};
    Frame state54B = {0x54B, 8, {0x5A, 0x08, 0x02, 0xD2, 0x00, 0x00, 0x00, 0x00}};
    Frame expect = {CLIMATE_STATE_FRAME_ID, 8, {0xBD, 0x01, 0x4E, 0x50, 0x00, 0x00, 0x00, 0x20}};
    assertTrue(checkStateFrames(&climate, state54A, state54B, expect));
}

test(ClimateStateTest, HalfManual) {
    INIT_STATE();
    Frame state54A = {0x54A, 8, {0xDA, 0x3E, 0x7F, 0x80, 0x47, 0x47, 0x00, 0x39}};
    Frame state54B = {0x54B, 8, {0x72, 0x0C, 0x00, 0xE4, 0x00, 0x00, 0x00, 0x00}};
    Frame expect = {CLIMATE_STATE_FRAME_ID, 8, {0x29, 0x00, 0x47, 0x47, 0x00, 0x00, 0x00, 0x39}};
    assertTrue(checkStateFrames(&climate, state54A, state54B, expect));
}

test(ClimateStateTest, FrontDefrostOn) {
    INIT_STATE();
    Frame state54A = {0x54A, 8, {0xDA, 0x3E, 0x7F, 0x80, 0x47, 0x47, 0x00, 0x22}};
    Frame state54B = {0x54B, 8, {0x5A, 0x34, 0x06, 0x24, 0x00, 0x00, 0x00, 0x02}};
    Frame expect = {CLIMATE_STATE_FRAME_ID, 8, {0x45, 0x03, 0x47, 0x47, 0x00, 0x00, 0x00, 0x22}};
    assertTrue(checkStateFrames(&climate, state54A, state54B, expect));
}

test(ClimateStateTest, FrontDefrostHalfManual) {
    INIT_STATE();
    Frame state54A = {0x54A, 8, {0xDA, 0x3E, 0x7F, 0x80, 0x47, 0x47, 0x00, 0x22}};
    Frame state54B = {0x54B, 8, {0x72, 0x34, 0x06, 0x24, 0x00, 0x00, 0x00, 0x02}};
    Frame expect = {CLIMATE_STATE_FRAME_ID, 8, {0x41, 0x03, 0x47, 0x47, 0x00, 0x00, 0x00, 0x22}};
    assertTrue(checkStateFrames(&climate, state54A, state54B, expect));
}

test(ClimateStateTest, RearDefrost) {
    INIT_STATE();
    Frame state54A = {0x54A, 8, {0x3C, 0x3E, 0x7F, 0x80, 0x00, 0x00, 0x00, 0x2B}};
    Frame state54B = {0x54B, 8, {0xF2, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x02}};
    Frame state625 = {0x625, 6, {0x33, 0x00, 0xFF, 0x9D, 0x00, 0x00}};
    Frame expect = {CLIMATE_STATE_FRAME_ID, 8, {0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x2B}};

    MockBroadcast cast(1, 0x5400);
    climate.send(state54A);
    climate.send(state54B);
    climate.send(state625);
    climate.receive(cast.impl);
    assertTrue(checkFrameCount(cast, 1) &&
        checkFrameEquals(cast.frames()[0], expect));
}

#endif  // __R51_TESTS_TEST_CLIMATE_STATE__
