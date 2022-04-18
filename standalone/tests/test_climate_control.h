#ifndef __R51_TESTS_TEST_CLIMATE_CONTROL__
#define __R51_TESTS_TEST_CLIMATE_CONTROL__

#include <AUnit.h>
#include <Arduino.h>
#include <Canny.h>
#include <Faker.h>

#include "mock_yield.h"
#include "src/binary.h"
#include "src/climate.h"
#include "src/config.h"
#include "testing.h"

using namespace aunit;
using ::Canny::Frame;


#define INIT_CONTROL(ACTIVE) \
    Faker::FakeClock clock;\
    Faker::FakeGPIO gpio;\
    Climate climate(&clock, &gpio);\
    initClimate(&climate, &clock, ACTIVE);
    
void initClimate(Climate* climate, Faker::FakeClock* clock, bool active = false) {
    Frame state54A(0x54A, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01});
    Frame state54B(0x54B, 0, {0xF2, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x02});
    if (active) {
        state54B.data()[0] = 0x59;
    }
    MockYield cast(2);
    clock->set(1000);
    climate->handle(state54A);
    climate->handle(state54B);
    climate->emit(cast.impl);
}

bool checkControlFrames(Climate* climate, const Frame& control, const Frame& expect540, const Frame& expect541) {
    MockYield cast(2, 0x540, 0xFFFFFFF0);
    climate->handle(control);
    climate->emit(cast.impl);
    return checkFrameCount(cast, 2) &&
        checkFrameEquals(cast.frames()[0], expect540) &&
        checkFrameEquals(cast.frames()[1], expect541);
}

bool checkNoControlFrames(Climate* climate, const Frame& control) {
    MockYield cast(2, 0x540, 0xFFFFFFF0);
    climate->handle(control);
    climate->emit(cast.impl);
    return checkFrameCount(cast, 0);
}

test(ClimateControlTest, Init) {
    Faker::FakeClock clock;
    Faker::FakeGPIO gpio;
    MockYield cast(2, 0x540, 0xFFFFFFF0);

    Frame init540(0x540, 0, {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    Frame init541(0x541, 0, {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    Frame ready540(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    Frame ready541(0x541, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    Climate climate(&clock, &gpio);
    for (int i = 0; i < 4; i++) {
        climate.emit(cast.impl);
        assertTrue(checkFrameCount(cast, 2) &&
            checkFrameEquals(cast.frames()[0], init540) &&
            checkFrameEquals(cast.frames()[1], init541));
        cast.reset();
        clock.delay(CLIMATE_CONTROL_INIT_HB);
    }
    climate.emit(cast.impl);
    assertTrue(checkFrameCount(cast, 2) &&
        checkFrameEquals(cast.frames()[0], ready540) &&
        checkFrameEquals(cast.frames()[1], ready541));
}

test(ClimateControlTest, Heartbeat) {
    INIT_CONTROL(true);
    Frame expect540, expect541;
    MockYield cast(2, 0x540, 0xFFFFFFF0);

    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    expect541 = Frame(0x541, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});

    clock.delay(CLIMATE_CONTROL_FRAME_HB);
    climate.emit(cast.impl);
    assertTrue(checkFrameCount(cast, 2) &&
        checkFrameEquals(cast.frames()[0], expect540) &&
        checkFrameEquals(cast.frames()[1], expect541));
}

test(ClimateControlTest, TriggerOff) {
    INIT_CONTROL(true);
    Frame control, expect540, expect541;

    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x84, 0x00});
    expect541 = Frame(0x541, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));

    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));
}

test(ClimateControlTest, TriggerAuto) {
    INIT_CONTROL(true);
    Frame control, expect540, expect541;

    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00});
    expect541 = Frame(0x541, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));

    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));
}

test(ClimateControlTest, TriggerAc) {
    INIT_CONTROL(true);
    Frame control, expect540, expect541;

    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x08, 0x04, 0x00});
    expect541 = Frame(0x541, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));

    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));
}

test(ClimateControlTest, TriggerDual) {
    INIT_CONTROL(true);
    Frame control, expect540, expect541;

    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x00});
    expect541 = Frame(0x541, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));

    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));
}

test(ClimateControlTest, TriggerMode) {
    INIT_CONTROL(true);
    Frame control, expect540, expect541;

    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00});
    expect541 = Frame(0x541, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));

    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));
}

test(ClimateControlTest, TriggerFrontDefrost) {
    INIT_CONTROL(true);
    Frame control, expect540, expect541;

    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00});
    expect541 = Frame(0x541, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));

    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));
}

test(ClimateControlTest, TriggerRecirculate) {
    INIT_CONTROL(true);
    Frame control, expect540, expect541;

    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    expect541 = Frame(0x541, 0, {0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));

    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect541 = Frame(0x541, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));
}

test(ClimateControlTest, TriggerRearDefrost) {
    INIT_CONTROL(true);

    Frame control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00});

    MockYield cast(0);

    assertEqual(gpio.pinMode(REAR_DEFROST_PIN), (uint32_t)OUTPUT);
    assertEqual(gpio.digitalRead(REAR_DEFROST_PIN), LOW);
    climate.handle(control);
    climate.emit(cast.impl);
    assertEqual(gpio.digitalRead(REAR_DEFROST_PIN), HIGH);

    clock.delay(REAR_DEFROST_TRIGGER_MS + 100);
    climate.emit(cast.impl);
    assertEqual(gpio.digitalRead(REAR_DEFROST_PIN), LOW);
}

test(ClimateControlTest, TriggerFanSpeedUp) {
    INIT_CONTROL(true);
    Frame control, expect540, expect541;

    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    expect541 = Frame(0x541, 0, {0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));

    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect541 = Frame(0x541, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));
}

test(ClimateControlTest, TriggerFanSpeedDown) {
    INIT_CONTROL(true);
    Frame control, expect540, expect541;

    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    expect541 = Frame(0x541, 0, {0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));

    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect541 = Frame(0x541, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));
}

test(ClimateControlTest, TriggerDriverTempWhenOn) {
    INIT_CONTROL(true);
    Frame control, expect540, expect541;

    // decrease temp
    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0xFF, 0x00, 0x20, 0x04, 0x00});
    expect541 = Frame(0x541, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));

    // increase temp
    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));

    // increase temp
    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x01, 0x00, 0x20, 0x04, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));

    // noop when both triggered
    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x01, 0x00, 0x20, 0x04, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));
}

test(ClimateControlTest, TriggerDriverTempWhenOff) {
    INIT_CONTROL(false);
    Frame control;

    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkNoControlFrames(&climate, control));

    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkNoControlFrames(&climate, control));
}

test(ClimateControlTest, TriggerPassengerTempWhenOn) {
    INIT_CONTROL(true);
    Frame control, expect540, expect541;

    // decrease temp
    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0xFF, 0x20, 0x04, 0x00});
    expect541 = Frame(0x541, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));

    // increase temp
    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));

    // increase temp
    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x01, 0x20, 0x04, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));

    // noop when both triggered
    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    expect540 = Frame(0x540, 0, {0x60, 0x40, 0x00, 0x00, 0x01, 0x20, 0x04, 0x00});
    assertTrue(checkControlFrames(&climate, control, expect540, expect541));
}

test(ClimateControlTest, TriggerPassengerTempWhenOff) {
    INIT_CONTROL(false);
    Frame control;

    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkNoControlFrames(&climate, control));

    control = Frame(CLIMATE_CONTROL_FRAME_ID, 0, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
    assertTrue(checkNoControlFrames(&climate, control));
}

#endif  // __R51_TESTS_TEST_CLIMATE_CONTROL__
