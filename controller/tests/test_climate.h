#ifndef __R51_TESTS_TEST_CLIMATE__
#define __R51_TESTS_TEST_CLIMATE__

#include <Arduino.h>
#include <AUnit.h>

#include "mock_broadcast.h"
#include "mock_clock.h"
#include "mock_gpio.h"
#include "src/climate.h"
#include "src/config.h"

using namespace aunit;


class ClimateTest : public TestOnce {
    public:
        MockClock clock;
        MockGPIO gpio;
        MockBroadcast cast;
        Climate* climate;

        ClimateTest() : cast(2) {}

        void setup() override {
            climate = new Climate(&clock, &gpio);
            cast.reset();
        }

        void teardown() override {
            delete climate;
        }
};

testF(ClimateTest, ControlInit) {
    cast.filter(0x540, 0xFFFFFFF0);

    Frame init540 = {
        .id = 0x540,
        .len = 8,
        .data = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    };
    Frame init541 = {
        .id = 0x541,
        .len = 8,
        .data = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    };
    Frame ready540 = {
        .id = 0x540,
        .len = 8,
        .data = {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00},
    };
    Frame ready541 = {
        .id = 0x541,
        .len = 8,
        .data = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    };

    for (int i = 0; i < 4; i++) {
        climate->receive(cast.impl);
        assertEqual(cast.count(), 2);
        assertTrue(frameEquals(cast.frames()[0], init540));
        assertTrue(frameEquals(cast.frames()[1], init541));
        cast.reset();
        clock.delay(CLIMATE_CONTROL_INIT_HB);
    }
    climate->receive(cast.impl);
    assertEqual(cast.count(), 2);
    assertTrue(frameEquals(cast.frames()[0], ready540));
    assertTrue(frameEquals(cast.frames()[1], ready541));
}

testF(ClimateTest, StateInit) {
    cast.filter(0x5400);

    Frame state54A = {
        .id = 0x54A,
        .len = 8,
        .data = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
    };
    Frame state54B = {
        .id = 0x54B,
        .len = 8,
        .data = {0xF2, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x02},
    };
    Frame ready5400 = {
        .id = 0x5400,
        .len = 8,
        .data = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
    };

    clock.delay(10);
    climate->receive(cast.impl);
    assertEqual(cast.count(), 0);
    climate->send(state54A);
    climate->send(state54B);

    clock.delay(10);
    climate->receive(cast.impl);
    assertEqual(cast.count(), 1);
    assertTrue(frameEquals(cast.frames()[0], ready5400));
}

#endif  // __R51_TESTS_TEST_CLIMATE__
