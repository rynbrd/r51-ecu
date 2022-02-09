#ifndef __R51_TESTS_TEST_CLIMATE_STATE__
#define __R51_TESTS_TEST_CLIMATE_STATE__

#include <Arduino.h>
#include <AUnit.h>

#include "mock_broadcast.h"
#include "mock_clock.h"
#include "mock_gpio.h"
#include "src/climate.h"
#include "testing.h"

using namespace aunit;


test(ClimateStateTest, Init) {
    MockClock clock;
    MockGPIO gpio;
    MockBroadcast cast(2, 0x5400);

    Frame state54A = {0x54A, 8, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}};
    Frame state54B = {0x54B, 8, {0xF2, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x02}};
    Frame ready5400 = {0x5400, 8, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}};

    Climate climate(&clock, &gpio);
    clock.delay(10);
    climate.receive(cast.impl);
    assertEqual(cast.count(), 0);
    climate.send(state54A);
    climate.send(state54B);

    clock.delay(10);
    climate.receive(cast.impl);
    assertEqual(cast.count(), 1);
    assertTrue(frameEquals(cast.frames()[0], ready5400));
}

/*
test(ClimateTest, StateHeartbeat) {
}
*/

#endif  // __R51_TESTS_TEST_CLIMATE_STATE__
