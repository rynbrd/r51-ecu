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


class MockStateBroadcast : public MockBroadcast {
    public:
        MockStateBroadcast(int capacity) : MockBroadcast(capacity) {}

        bool filter(const Frame& frame) {
            return frame.id == 0x5400;
        }
};

test(ClimateTest, ControlInit) {
    MockClock clock;
    MockGPIO gpio;
    MockBroadcast cast(2);

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

    Climate climate(&clock, &gpio);
    for (int i = 0; i < 4; i++) {
        climate.receive(cast.impl);
        assertEqual(cast.count(), 2);
        assertTrue(frameEquals(cast.frames()[0], init540));
        assertTrue(frameEquals(cast.frames()[1], init541));
        cast.reset();
        clock.delay(CLIMATE_CONTROL_INIT_HB);
    }
    climate.receive(cast.impl);
    assertEqual(cast.count(), 2);
    assertTrue(frameEquals(cast.frames()[0], ready540));
    assertTrue(frameEquals(cast.frames()[1], ready541));
}

test(CliamteTest, StateInit) {
    MockClock clock;
    MockGPIO gpio;
    MockStateBroadcast cast(1);

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

#endif  // __R51_TESTS_TEST_CLIMATE__
