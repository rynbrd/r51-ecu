#ifndef __R51_TESTS_TEST_SETTINGS__
#define __R51_TESTS_TEST_SETTINGS__

#include <Arduino.h>
#include <AUnit.h>

#include "mock_broadcast.h"
#include "mock_clock.h"
#include "src/binary.h"
#include "src/settings.h"
#include "testing.h"

using namespace aunit;


class SettingsTest : public TestOnce {
    public:
        SettingsTest() : TestOnce(), clock() {}

        void setup() override {
            TestOnce::setup();
            clock.set(0);
        }

        inline uint32_t responseId(uint32_t request_id) {
            return (request_id & ~0x010) | 0x020;
        }

        void fillFrame(Frame* frame, uint32_t id, byte a, byte b = 0xFF, byte c = 0xFF,
                byte d = 0xFF, byte e = 0xFF, byte f = 0xFF, byte g = 0xFF, byte h = 0xFF) {
            frame->id = id;
            frame->len = 8;
            frame->data[0] = a;
            frame->data[1] = b;
            frame->data[2] = c;
            frame->data[3] = d;
            frame->data[4] = e;
            frame->data[5] = f;
            frame->data[6] = g;
            frame->data[7] = h;
        }

        void fillEnterRequest(Frame* frame, uint32_t id) {
            fillFrame(frame, id, 0x02, 0x10, 0xC0);
        }

        void fillEnterResponse(Frame* frame, uint32_t id) {
            fillFrame(frame, id, 0x02, 0x50, 0xC0);
        }

        void fillExitRequest(Frame* frame, uint32_t id) {
            fillFrame(frame, id, 0x02, 0x10, 0x81);
        }

        void fillExitResponse(Frame* frame, uint32_t id) {
            fillFrame(frame, id, 0x02, 0x50, 0x81);
        }

        void fillUpdateRequest(Frame* frame, uint32_t id, uint8_t command, uint8_t value) {
            fillFrame(frame, id, 0x03, 0x3B, command, value);
        }

        void fillUpdateResponse(Frame* frame, uint32_t id, uint8_t command) {
            fillFrame(frame, id, 0x02, 0x7B, command);
        }

        void fillResetRequest(Frame* frame, uint32_t id) {
            fillFrame(frame, id, 0x03, 0x3B, 0x1F, 0x00);
        }

        void fillResetResponse(Frame* frame, uint32_t id) {
            fillFrame(frame, id, 0x02, 0x7B, 0x1F);
        }

        void fillState0221Request(Frame* frame, uint32_t id) {
            fillFrame(frame, id, 0x02, 0x21, 0x01);
        }

        void fillState3000Request(Frame* frame, uint32_t id) {
            fillFrame(frame, id, 0x30, 0x00, 0x0A);
        }

        bool checkUpdate(Settings* settings, const Frame& control,
                uint32_t expect_id, uint8_t expect_command, uint8_t expect_value,
                const Frame* state_71E_10, const Frame* state_71E_21,
                const Frame* state_71E_22, const Frame* state_71F_05) {
            MockBroadcast cast(1, 0x700, 0xFFFFFF00);
            Frame frame;

            // Send control frame to perform update.
            settings->send(control);

            // Exchange enter frames.
            settings->receive(cast.impl);
            fillEnterRequest(&frame, expect_id);
            if (!checkFrameCount(cast, 1) ||
                !checkFrameEquals(cast.frames()[0], frame)) {
                return false;
            }
            cast.reset();
            fillEnterResponse(&frame, responseId(expect_id));
            settings->send(frame);

            // Exchange update frames.
            settings->receive(cast.impl);
            fillUpdateRequest(&frame, expect_id, expect_command, expect_value);
            if (!checkFrameCount(cast, 1) ||
                !checkFrameEquals(cast.frames()[0], frame)) {
                return false;
            }
            cast.reset();
            fillUpdateResponse(&frame, responseId(expect_id), expect_command);
            settings->send(frame);

            if (expect_id == 0x71E) {
                // Exchange initial state frames.
                settings->receive(cast.impl);
                fillState0221Request(&frame, 0x71E);
                if (!checkFrameCount(cast, 1) ||
                    !checkFrameEquals(cast.frames()[0], frame)) {
                    return false;
                }
                cast.reset();
                settings->send(*state_71E_10);
                
                // Exchange secondary state frames.
                settings->receive(cast.impl);
                fillState3000Request(&frame, 0x71E);
                if (!checkFrameCount(cast, 1) ||
                    !checkFrameEquals(cast.frames()[0], frame)) {
                    return false;
                }
                cast.reset();
                settings->send(*state_71E_21);
                settings->send(*state_71E_22);
            } else if (expect_id == 0x71F) {
                // Exchange state frames.
                settings->receive(cast.impl);
                fillState0221Request(&frame, 0x71F);
                if (!checkFrameCount(cast, 1) ||
                    !checkFrameEquals(cast.frames()[0], frame)) {
                    return false;
                }
                cast.reset();
                settings->send(*state_71F_05);
            }

            // Exchange exit frames.
            settings->receive(cast.impl);
            fillExitRequest(&frame, expect_id);
            if (!checkFrameCount(cast, 1) ||
                !checkFrameEquals(cast.frames()[0], frame)) {
                return false;
            }
            cast.reset();
            fillExitResponse(&frame, responseId(expect_id));
            settings->send(frame);

            return true;
        }

        bool checkUpdate(Settings* settings, const Frame& control,
                uint32_t expect_id, uint8_t expect_command, uint8_t expect_value,
                const Frame& state_71E_10, const Frame& state_71E_21,
                const Frame& state_71E_22) {
            return checkUpdate(settings, control, expect_id, expect_command, expect_value,
                    &state_71E_10, &state_71E_21, &state_71E_22, nullptr);
        }

        bool checkUpdate(Settings* settings, const Frame& control,
                uint32_t expect_id, uint8_t expect_command, uint8_t expect_value,
                const Frame& state_71F_05) {
            return checkUpdate(settings, control, expect_id, expect_command, expect_value,
                    nullptr, nullptr, nullptr, &state_71F_05);
        }

        bool checkNoop(Settings* settings, const Frame& control) {
            MockBroadcast cast(1, 0x700, 0xFFFFFF00);
            settings->send(control);
            return checkFrameCount(cast, 0);
        }

        MockClock clock;
};

testF(SettingsTest, Init) {
    MockBroadcast cast(2, 0x700, 0xFFFFFF00);
    Settings settings(&clock);

    Frame frameE;
    Frame frameF;

    // Trigger settings initialization.
    assertTrue(settings.init());
    settings.receive(cast.impl);

    // Receive enter frames.
    fillEnterRequest(&frameE, 0x71E);
    fillEnterRequest(&frameF, 0x71F);
    assertTrue(checkFrameCount(cast, 2) &&
        checkFrameEquals(cast.frames()[0], frameE) &&
        checkFrameEquals(cast.frames()[1], frameF));
    cast.reset();
    // Simulate response.
    fillEnterResponse(&frameE, 0x72E);
    settings.send(frameE);
    fillEnterResponse(&frameF, 0x72F);
    settings.send(frameF);

    // Receive next init frames.
    settings.receive(cast.impl);
    fillFrame(&frameE, 0x71E, 0x02, 0x3B, 0x00);
    fillFrame(&frameF, 0x71F, 0x02, 0x3B, 0x00);
    assertTrue(checkFrameCount(cast, 2) &&
        checkFrameEquals(cast.frames()[0], frameE) &&
        checkFrameEquals(cast.frames()[1], frameF));
    cast.reset();
    // Simulate response.
    fillFrame(&frameE, 0x72E, 0x06, 0x7B, 0x00, 0x60, 0x01, 0x0E, 0x07);
    settings.send(frameE);
    fillFrame(&frameF, 0x72F, 0x06, 0x7B, 0x00, 0x60, 0x01, 0x0E, 0x07);
    settings.send(frameF);

    // Receive next init frames. Final F frame.
    settings.receive(cast.impl);
    fillFrame(&frameE, 0x71E, 0x02, 0x3B, 0x20);
    fillExitRequest(&frameF, 0x71F);
    assertTrue(checkFrameCount(cast, 2) &&
        checkFrameEquals(cast.frames()[0], frameE) &&
        checkFrameEquals(cast.frames()[1], frameF));
    cast.reset();
    // Simulate response.
    fillFrame(&frameE, 0x72E, 0x06, 0x7B, 0x20, 0xC2, 0x6F, 0x73, 0xD3);
    settings.send(frameE);
    fillExitResponse(&frameF, 0x72F);
    settings.send(frameF);

    // Receive next E init Frame.
    settings.receive(cast.impl);
    fillFrame(&frameE, 0x71E, 0x02, 0x3B, 0x40);
    assertTrue(checkFrameCount(cast, 1) &&
        checkFrameEquals(cast.frames()[0], frameE));
    cast.reset();
    // Simulate response.
    fillFrame(&frameE, 0x72E, 0x06, 0x7B, 0x40, 0xC2, 0xA1, 0x90, 0x01);
    settings.send(frameE);

    // Receive next E init Frame.
    settings.receive(cast.impl);
    fillFrame(&frameE, 0x71E, 0x02, 0x3B, 0x60);
    assertTrue(checkFrameCount(cast, 1) &&
        checkFrameEquals(cast.frames()[0], frameE));
    cast.reset();
    // Simulate response.
    fillFrame(&frameE, 0x72E, 0x06, 0x7B, 0x60, 0x00, 0xFF, 0xF1, 0x70);
    settings.send(frameE);
}

testF(SettingsTest, Retrieve) {
    MockBroadcast cast(3);
    Settings settings(&clock);

    Frame frameE;
    Frame frameF;
    Frame state;

    // Send control frame to trigger retrieve.
    Frame control = {0x5701, 8, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}};
    settings.send(control);

    // Receive enter request frames.
    settings.receive(cast.impl);
    fillEnterRequest(&frameE, 0x71E);
    fillEnterRequest(&frameF, 0x71F);
    assertTrue(checkFrameCount(cast, 2) &&
        checkFrameEquals(cast.frames()[0], frameE) &&
        checkFrameEquals(cast.frames()[1], frameF));
    cast.reset();
    // Simulate response.
    fillEnterResponse(&frameE, 0x72E);
    settings.send(frameE);
    fillEnterResponse(&frameF, 0x72F);
    settings.send(frameF);

    // Receive initial state frames.
    settings.receive(cast.impl);
    fillState0221Request(&frameE, 0x71E);
    fillState0221Request(&frameF, 0x71F);
    assertTrue(checkFrameCount(cast, 2) &&
        checkFrameEquals(cast.frames()[0], frameE) &&
        checkFrameEquals(cast.frames()[1], frameF));
    cast.reset();
    // Simulate response.
    fillFrame(&frameE, 0x72E, 0x10, 0x11, 0x61, 0x01, 0x00, 0x1E, 0x24, 0x00);
    settings.send(frameE);
    fillFrame(&frameF, 0x72F, 0x05, 0x61, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF);
    settings.send(frameF);

    // Receive next E state frame and F exit frame.
    settings.receive(cast.impl);
    fillState3000Request(&frameE, 0x71E);
    fillExitRequest(&frameF, 0x71F);
    fillFrame(&state, 0x5700, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    assertTrue(checkFrameCount(cast, 3) &&
        checkFrameEquals(cast.frames()[0], frameE) &&
        checkFrameEquals(cast.frames()[1], frameF) &&
        checkFrameEquals(cast.frames()[2], state));
    cast.reset();
    // Simulate response.
    fillFrame(&frameE, 0x72E, 0x21, 0x10, 0x0C, 0x40, 0x40, 0x01, 0x64, 0x00);
    settings.send(frameE);
    fillFrame(&frameE, 0x72E, 0x22, 0x94, 0x00, 0x00, 0x47, 0xFF, 0xFF, 0xFF);
    settings.send(frameE);
    fillExitResponse(&frameF, 0x72F);
    settings.send(frameF);

    // Receive E exit frame.
    settings.receive(cast.impl);
    fillExitRequest(&frameE, 0x71E);
    fillFrame(&state, 0x5700, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    assertTrue(checkFrameCount(cast, 2) &&
        checkFrameEquals(cast.frames()[0], frameE) &&
        checkFrameEquals(cast.frames()[1], state));
    cast.reset();
    // Simulate response.
    fillExitResponse(&frameE, 0x72E);
    settings.send(frameE);
}

testF(SettingsTest, ResetToDefault) {
    MockBroadcast cast(3);
    Settings settings(&clock);

    Frame frameE;
    Frame frameF;
    Frame state;

    // Send control frame to trigger reset.
    Frame control = {0x5701, 8, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80}};
    settings.send(control);

    // Receive enter request frames.
    settings.receive(cast.impl);
    fillEnterRequest(&frameE, 0x71E);
    fillEnterRequest(&frameF, 0x71F);
    assertTrue(checkFrameCount(cast, 2) &&
        checkFrameEquals(cast.frames()[0], frameE) &&
        checkFrameEquals(cast.frames()[1], frameF));
    cast.reset();
    // Simulate response.
    fillEnterResponse(&frameE, 0x72E);
    settings.send(frameE);
    fillEnterResponse(&frameF, 0x72F);
    settings.send(frameF);

    // Send reset all command.
    settings.receive(cast.impl);
    fillResetRequest(&frameE, 0x71E);
    fillResetRequest(&frameF, 0x71F);
    assertTrue(checkFrameCount(cast, 2) &&
        checkFrameEquals(cast.frames()[0], frameE) &&
        checkFrameEquals(cast.frames()[1], frameF));
    cast.reset();
    // Simulate response.
    fillResetResponse(&frameE, 0x72E);
    settings.send(frameE);
    fillResetResponse(&frameF, 0x72F);
    settings.send(frameF);

    // Receive initial state frames.
    settings.receive(cast.impl);
    fillState0221Request(&frameE, 0x71E);
    fillState0221Request(&frameF, 0x71F);
    assertTrue(checkFrameCount(cast, 2) &&
        checkFrameEquals(cast.frames()[0], frameE) &&
        checkFrameEquals(cast.frames()[1], frameF));
    cast.reset();
    // Simulate response.
    fillFrame(&frameE, 0x72E, 0x10, 0x11, 0x61, 0x01, 0x00, 0x1E, 0x24, 0x00);
    settings.send(frameE);
    fillFrame(&frameF, 0x72F, 0x05, 0x61, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF);
    settings.send(frameF);

    // Receive next E state frame and F exit frame.
    settings.receive(cast.impl);
    fillState3000Request(&frameE, 0x71E);
    fillExitRequest(&frameF, 0x71F);
    fillFrame(&state, 0x5700, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    assertTrue(checkFrameCount(cast, 3) &&
        checkFrameEquals(cast.frames()[0], frameE) &&
        checkFrameEquals(cast.frames()[1], frameF) &&
        checkFrameEquals(cast.frames()[2], state));
    cast.reset();
    // Simulate response.
    fillFrame(&frameE, 0x72E, 0x21, 0x10, 0x0C, 0x40, 0x40, 0x01, 0x64, 0x00);
    settings.send(frameE);
    fillFrame(&frameE, 0x72E, 0x22, 0x94, 0x00, 0x00, 0x47, 0xFF, 0xFF, 0xFF);
    settings.send(frameE);
    fillExitResponse(&frameF, 0x72F);
    settings.send(frameF);

    // Receive E exit frame.
    settings.receive(cast.impl);
    fillExitRequest(&frameE, 0x71E);
    fillFrame(&state, 0x5700, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    assertTrue(checkFrameCount(cast, 2) &&
        checkFrameEquals(cast.frames()[0], frameE) &&
        checkFrameEquals(cast.frames()[1], state));
    cast.reset();
    // Simulate response.
    fillExitResponse(&frameE, 0x72E);
    settings.send(frameE);
}

testF(SettingsTest, ToggleAutoInteriorIllumination) {
    Settings settings(&clock);

    // Initial state.
    Frame control = {0x5701, 8, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    Frame state10 = {0x72E, 8, {0x10, 0x11, 0x61, 0x01, 0x00, 0x1E, 0x24, 0x00}};
    Frame state21 = {0x72E, 8, {0x21, 0x10, 0x0C, 0x40, 0x40, 0x01, 0x64, 0x00}};
    Frame state22 = {0x72E, 8, {0x22, 0x94, 0x00, 0x00, 0x47, 0xFF, 0xFF, 0xFF}};

    // Toggle setting on.
    toggleBit(control.data, 0, 0);
    setBit(state10.data, 4, 5, 1);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x10, 0x01, state10, state21, state22));

    // Toggle setting off.
    toggleBit(control.data, 0, 0);
    setBit(state10.data, 4, 5, 0);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x10, 0x00, state10, state21, state22));

    // Toggle setting on.
    toggleBit(control.data, 0, 0);
    setBit(state10.data, 4, 5, 1);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x10, 0x01, state10, state21, state22));
}

testF(SettingsTest, ToggleSlideDriverSeat) {
    Settings settings(&clock);

    // Initial state.
    Frame control = {0x5701, 8, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    Frame state05 = {0x72F, 8, {0x05, 0x61, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF}};

    // Toggle setting on.
    toggleBit(control.data, 0, 1);
    setBit(state05.data, 3, 0, 1);
    assertTrue(checkUpdate(&settings, control, 0x71F, 0x01, 0x01, state05));

    // Toggle setting off.
    toggleBit(control.data, 0, 1);
    setBit(state05.data, 3, 0, 0);
    assertTrue(checkUpdate(&settings, control, 0x71F, 0x01, 0x00, state05));

    // Toggle setting on.
    toggleBit(control.data, 0, 1);
    setBit(state05.data, 3, 0, 1);
    assertTrue(checkUpdate(&settings, control, 0x71F, 0x01, 0x01, state05));
}

testF(SettingsTest, ToggleSpeedSendingWipers) {
    Settings settings(&clock);

    // Initial state.
    Frame control = {0x5701, 8, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    Frame state10 = {0x72E, 8, {0x10, 0x11, 0x61, 0x01, 0x00, 0x1E, 0x24, 0x00}};
    Frame state21 = {0x72E, 8, {0x21, 0x10, 0x0C, 0x40, 0x40, 0x01, 0x64, 0x00}};
    Frame state22 = {0x72E, 8, {0x22, 0x94, 0x00, 0x00, 0x47, 0xFF, 0xFF, 0xFF}};

    // Toggle setting on.
    toggleBit(control.data, 0, 2);
    setBit(state22.data, 1, 7, 0);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x47, 0x00, state10, state21, state22));

    // Toggle setting off.
    toggleBit(control.data, 0, 2);
    setBit(state22.data, 1, 7, 1);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x47, 0x01, state10, state21, state22));

    // Toggle setting on.
    toggleBit(control.data, 0, 2);
    setBit(state22.data, 1, 7, 0);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x47, 0x00, state10, state21, state22));
}

testF(SettingsTest, AutoHeadlighSensitivity) {
    Settings settings(&clock);

    // Initial state.
    Frame control = {0x5701, 8, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    Frame state10 = {0x72E, 8, {0x10, 0x11, 0x61, 0x01, 0x00, 0x1E, 0x24, 0x00}};
    Frame state21 = {0x72E, 8, {0x21, 0x10, 0x0C, 0x40, 0x40, 0x01, 0x64, 0x00}};
    Frame state22 = {0x72E, 8, {0x22, 0x94, 0x00, 0x00, 0x47, 0xFF, 0xFF, 0xFF}};

    // Increase setting.
    toggleBit(control.data, 1, 0);
    state21.data[2] = 0x00;
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x37, 0x00, state10, state21, state22));

    // Increase setting.
    toggleBit(control.data, 1, 0);
    state21.data[2] = (0x01 << 2);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x37, 0x01, state10, state21, state22));

    // Increase setting.
    toggleBit(control.data, 1, 0);
    state21.data[2] = (0x02 << 2);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x37, 0x02, state10, state21, state22));

    // Increase setting when at max.
    toggleBit(control.data, 1, 0);
    assertTrue(checkNoop(&settings, control));

    // Decrease setting.
    toggleBit(control.data, 1, 1);
    state21.data[2] = (0x01 << 2);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x37, 0x01, state10, state21, state22));

    // Decrease setting.
    toggleBit(control.data, 1, 1);
    state21.data[2] = 0x00;
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x37, 0x00, state10, state21, state22));

    // Decrease setting.
    toggleBit(control.data, 1, 1);
    state21.data[2] = (0x03 << 2);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x37, 0x03, state10, state21, state22));

    // Decrease setting when at min.
    toggleBit(control.data, 1, 1);
    assertTrue(checkNoop(&settings, control));
}

void setAutoHeadlightDelayState(Frame* state21, byte value) {
    state21->data[2] = (value >> 2) & 0x01;
    state21->data[3] = (value & 0x03) << 6;
}

testF(SettingsTest, AutoHeadlightOffDelay) {
    Settings settings(&clock);

    // Initial state.
    byte value = 0x00;
    Frame control = {0x5701, 8, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    Frame state10 = {0x72E, 8, {0x10, 0x11, 0x61, 0x01, 0x00, 0x1E, 0x24, 0x00}};
    Frame state21 = {0x72E, 8, {0x21, 0x10, 0x0C, 0x40, 0x40, 0x01, 0x64, 0x00}};
    Frame state22 = {0x72E, 8, {0x22, 0x94, 0x00, 0x00, 0x47, 0xFF, 0xFF, 0xFF}};

    // Increase setting to 30s.
    toggleBit(control.data, 1, 4);
    value = 0x02;
    setAutoHeadlightDelayState(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22));

    // Increase setting to 45s (default).
    toggleBit(control.data, 1, 4);
    value = 0x00;
    setAutoHeadlightDelayState(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22));

    // Increase setting to 60s.
    toggleBit(control.data, 1, 4);
    value = 0x03;
    setAutoHeadlightDelayState(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22));

    // Increase setting to 90s.
    toggleBit(control.data, 1, 4);
    value = 0x04;
    setAutoHeadlightDelayState(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22));

    // Increase setting to 120s.
    toggleBit(control.data, 1, 4);
    value = 0x05;
    setAutoHeadlightDelayState(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22));

    // Increase setting to 150s.
    toggleBit(control.data, 1, 4);
    value = 0x06;
    setAutoHeadlightDelayState(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22));

    // Increase setting to 180s.
    toggleBit(control.data, 1, 4);
    value = 0x07;
    setAutoHeadlightDelayState(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22));

    // Increase setting when at max.
    toggleBit(control.data, 1, 4);
    assertTrue(checkNoop(&settings, control));

    // Decrease setting to 150s.
    toggleBit(control.data, 1, 5);
    value = 0x06;
    setAutoHeadlightDelayState(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22));

    // Decrease setting to 120s.
    toggleBit(control.data, 1, 5);
    value = 0x05;
    setAutoHeadlightDelayState(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22));

    // Decrease setting to 90s.
    toggleBit(control.data, 1, 5);
    value = 0x04;
    setAutoHeadlightDelayState(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22));

    // Decrease setting to 60s.
    toggleBit(control.data, 1, 5);
    value = 0x03;
    setAutoHeadlightDelayState(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22));

    // Decrease setting to 45s (default).
    toggleBit(control.data, 1, 5);
    value = 0x00;
    setAutoHeadlightDelayState(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22));

    // Decrease setting to 30s.
    toggleBit(control.data, 1, 5);
    value = 0x02;
    setAutoHeadlightDelayState(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22));

    // Decrease setting to 0s.
    toggleBit(control.data, 1, 5);
    value = 0x01;
    setAutoHeadlightDelayState(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22));

    // Decrease setting when at min.
    toggleBit(control.data, 1, 5);
    assertTrue(checkNoop(&settings, control));
}

testF(SettingsTest, ToggleSelectiveDoorUnlock) {
    Settings settings(&clock);

    // Initial state.
    Frame control = {0x5701, 8, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    Frame state10 = {0x72E, 8, {0x10, 0x11, 0x61, 0x01, 0x00, 0x1E, 0x24, 0x00}};
    Frame state21 = {0x72E, 8, {0x21, 0x10, 0x0C, 0x40, 0x40, 0x01, 0x64, 0x00}};
    Frame state22 = {0x72E, 8, {0x22, 0x94, 0x00, 0x00, 0x47, 0xFF, 0xFF, 0xFF}};

    // Toggle setting on.
    toggleBit(control.data, 2, 0);
    setBit(state10.data, 4, 7, 1);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x02, 0x01, state10, state21, state22));

    // Toggle setting off.
    toggleBit(control.data, 2, 0);
    setBit(state10.data, 4, 7, 0);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x02, 0x00, state10, state21, state22));

    // Toggle setting on.
    toggleBit(control.data, 2, 0);
    setBit(state10.data, 4, 7, 1);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x02, 0x01, state10, state21, state22));
}

void setAutoReLockTime(Frame* state21, byte value) {
    state21->data[1] = (value & 0x03) << 4;
}

testF(SettingsTest, AutoReLockTime) {
    Settings settings(&clock);

    // Initial state.
    byte value = 0x00;
    Frame control = {0x5701, 8, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    Frame state10 = {0x72E, 8, {0x10, 0x11, 0x61, 0x01, 0x00, 0x1E, 0x24, 0x00}};
    Frame state21 = {0x72E, 8, {0x21, 0x10, 0x0C, 0x40, 0x40, 0x01, 0x64, 0x00}};
    Frame state22 = {0x72E, 8, {0x22, 0x94, 0x00, 0x00, 0x47, 0xFF, 0xFF, 0xFF}};

    // Increase setting to 1m (default).
    toggleBit(control.data, 2, 4);
    value = 0x00;
    setAutoReLockTime(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x2F, value, state10, state21, state22));

    // Increase setting to 5m.
    toggleBit(control.data, 2, 4);
    value = 0x02;
    setAutoReLockTime(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x2F, value, state10, state21, state22));

    // Increase setting when at max.
    toggleBit(control.data, 2, 4);
    assertTrue(checkNoop(&settings, control));

    // Decrease setting to 1m (default).
    toggleBit(control.data, 2, 5);
    value = 0x00;
    setAutoReLockTime(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x2F, value, state10, state21, state22));

    // Decrease setting to off.
    toggleBit(control.data, 2, 5);
    value = 0x01;
    setAutoReLockTime(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x2F, value, state10, state21, state22));

    // Decrease setting when at min.
    toggleBit(control.data, 2, 5);
    assertTrue(checkNoop(&settings, control));
}

testF(SettingsTest, RemoteKeyResponseHorn) {
    Settings settings(&clock);

    // Initial state.
    Frame control = {0x5701, 8, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    Frame state10 = {0x72E, 8, {0x10, 0x11, 0x61, 0x01, 0x00, 0x1E, 0x24, 0x00}};
    Frame state21 = {0x72E, 8, {0x21, 0x10, 0x0C, 0x40, 0x40, 0x01, 0x64, 0x00}};
    Frame state22 = {0x72E, 8, {0x22, 0x94, 0x00, 0x00, 0x47, 0xFF, 0xFF, 0xFF}};

    // Toggle setting on.
    toggleBit(control.data, 3, 0);
    setBit(state10.data, 7, 3, 1);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x2A, 0x01, state10, state21, state22));

    // Toggle setting off.
    toggleBit(control.data, 3, 0);
    setBit(state10.data, 7, 3, 0);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x2A, 0x00, state10, state21, state22));

    // Toggle setting on.
    toggleBit(control.data, 3, 0);
    setBit(state10.data, 7, 3, 1);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x2A, 0x01, state10, state21, state22));
}

void setRemoteKeyResponseLights(Frame* state21, byte value) {
    state21->data[1] = 0x10 | ((value & 0x03) << 6);
}

testF(SettingsTest, RemoteKeyResponseLights) {
    Settings settings(&clock);

    // Initial state.
    byte value = 0x00;
    Frame control = {0x5701, 8, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    Frame state10 = {0x72E, 8, {0x10, 0x11, 0x61, 0x01, 0x00, 0x1E, 0x24, 0x00}};
    Frame state21 = {0x72E, 8, {0x21, 0x10, 0x0C, 0x40, 0x40, 0x01, 0x64, 0x00}};
    Frame state22 = {0x72E, 8, {0x22, 0x94, 0x00, 0x00, 0x47, 0xFF, 0xFF, 0xFF}};

    // Increase setting to "unlock".
    toggleBit(control.data, 3, 2);
    value = 0x01;
    setRemoteKeyResponseLights(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x2E, value, state10, state21, state22));

    // Increase setting to "lock".
    toggleBit(control.data, 3, 2);
    value = 0x02;
    setRemoteKeyResponseLights(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x2E, value, state10, state21, state22));

    // Increase setting to "on".
    toggleBit(control.data, 3, 2);
    value = 0x03;
    setRemoteKeyResponseLights(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x2E, value, state10, state21, state22));

    // Increase setting when at max.
    toggleBit(control.data, 3, 2);
    assertTrue(checkNoop(&settings, control));

    // Decrease setting to "lock".
    toggleBit(control.data, 3, 3);
    value = 0x02;
    setRemoteKeyResponseLights(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x2E, value, state10, state21, state22));

    // Decrease setting to "unlock".
    toggleBit(control.data, 3, 3);
    value = 0x01;
    setRemoteKeyResponseLights(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x2E, value, state10, state21, state22));

    // Decrease setting to "off".
    toggleBit(control.data, 3, 3);
    value = 0x00;
    setRemoteKeyResponseLights(&state21, value);
    assertTrue(checkUpdate(&settings, control, 0x71E, 0x2E, value, state10, state21, state22));

    // Decrease setting when at min.
    toggleBit(control.data, 3, 3);
    assertTrue(checkNoop(&settings, control));
}

#endif  // __R51_TESTS_TEST_SETTINGS__
