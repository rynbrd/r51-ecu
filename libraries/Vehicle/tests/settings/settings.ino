#include <AUnit.h>
#include <Arduino.h>
#include <Faker.h>
#include <Foundation.h>
#include <Test.h>
#include <Vehicle.h>

namespace R51 {

using namespace aunit;
using ::Canny::Frame;
using ::Faker::FakeClock;

class SettingsTest : public TestOnce {
    public:
        Faker::FakeClock clock;

        SettingsTest() : TestOnce(), clock() {}

        void setup() override {
            TestOnce::setup();
            clock.set(0);
        }

        inline uint32_t responseId(uint32_t request_id) {
            return (request_id & ~0x010) | 0x020;
        }

        template <size_t N> 
        void fillFrame(Frame* frame, uint32_t id, const uint8_t (&data)[N]) {
            frame->clear(0xFF);
            frame->id(id);
            frame->data(data);
            frame->resize(8);
            for (int i = N; i < 8; i++) {
                frame->data()[i] = 0xFF;
            }
        }

        void fillEnterRequest(Frame* frame, uint32_t id) {
            fillFrame(frame, id, {0x02, 0x10, 0xC0});
        }

        void fillEnterResponse(Frame* frame, uint32_t id) {
            fillFrame(frame, id, {0x02, 0x50, 0xC0});
        }

        void fillExitRequest(Frame* frame, uint32_t id) {
            fillFrame(frame, id, {0x02, 0x10, 0x81});
        }

        void fillExitResponse(Frame* frame, uint32_t id) {
            fillFrame(frame, id, {0x02, 0x50, 0x81});
        }

        void fillUpdateRequest(Frame* frame, uint32_t id, uint8_t command, uint8_t value) {
            fillFrame(frame, id, {0x03, 0x3B, command, value});
        }

        void fillUpdateResponse(Frame* frame, uint32_t id, uint8_t command) {
            fillFrame(frame, id, {0x02, 0x7B, command});
        }

        void fillResetRequest(Frame* frame, uint32_t id) {
            fillFrame(frame, id, {0x03, 0x3B, 0x1F, 0x00});
        }

        void fillResetResponse(Frame* frame, uint32_t id) {
            fillFrame(frame, id, {0x02, 0x7B, 0x1F});
        }

        void fillState0221Request(Frame* frame, uint32_t id) {
            fillFrame(frame, id, {0x02, 0x21, 0x01});
        }

        void fillState3000Request(Frame* frame, uint32_t id) {
            fillFrame(frame, id, {0x30, 0x00, 0x0A});
        }

        void setAutoHeadlightDelayState(Frame* state21, byte value) {
            state21->data()[2] = (value >> 2) & 0x01;
            state21->data()[3] = (value & 0x03) << 6;
        }

        void setAutoReLockTime(Frame* state21, byte value) {
            state21->data()[1] = (value & 0x03) << 4;
        }

        void setRemoteKeyResponseLights(Frame* state21, byte value) {
            state21->data()[1] = 0x10 | ((value & 0x03) << 6);
        }

        void checkUpdate(Settings* settings, Event& control,
                uint32_t expect_id, uint8_t expect_command, uint8_t expect_value,
                Frame* state_71E_10, Frame* state_71E_21,
                Frame* state_71E_22, Frame* state_71F_05,
                Event* expect_event) {
            FakeYield yield;
            Frame frame;

            // Send control event to perform update.
            settings->handle(MessageView(&control), yield);
            yield.clear();

            // Exchange enter frames.
            settings->emit(yield);
            fillEnterRequest(&frame, expect_id);
            assertSize(yield, 1);
            assertIsCANFrame(yield.messages()[0], frame);
            yield.clear();
            fillEnterResponse(&frame, responseId(expect_id));
            settings->handle(MessageView(&frame), yield);
            yield.clear();

            // Exchange update frames.
            settings->emit(yield);
            fillUpdateRequest(&frame, expect_id, expect_command, expect_value);
            assertSize(yield, 1);
            assertIsCANFrame(yield.messages()[0], frame);
            yield.clear();
            fillUpdateResponse(&frame, responseId(expect_id), expect_command);
            settings->handle(MessageView(&frame), yield);
            yield.clear();

            if (expect_id == 0x71E) {
                // Exchange initial state frames.
                settings->emit(yield);
                fillState0221Request(&frame, 0x71E);
                assertSize(yield, 1);
                assertIsCANFrame(yield.messages()[0], frame);
                yield.clear();
                settings->handle(MessageView(state_71E_10), yield);
                yield.clear();
                
                // Exchange secondary state frames.
                settings->emit(yield);
                fillState3000Request(&frame, 0x71E);
                assertSize(yield, 1);
                assertIsCANFrame(yield.messages()[0], frame);
                yield.clear();
                settings->handle(MessageView(state_71E_21), yield);
                settings->handle(MessageView(state_71E_22), yield);
                yield.clear();
            } else if (expect_id == 0x71F) {
                // Exchange state frames.
                settings->emit(yield);
                fillState0221Request(&frame, 0x71F);
                assertSize(yield, 1);
                assertIsCANFrame(yield.messages()[0], frame);
                yield.clear();
                settings->handle(MessageView(state_71F_05), yield);
                yield.clear();
            }

            // Exchange exit frames.
            settings->emit(yield);
            fillExitRequest(&frame, expect_id);
            assertSize(yield, 1);
            assertIsCANFrame(yield.messages()[0], frame);
            yield.clear();
            fillExitResponse(&frame, responseId(expect_id));
            settings->handle(MessageView(&frame), yield);
            yield.clear();

            // Check resulting state frame.
            if (expect_event != nullptr) {
                settings->emit(yield);
                assertSize(yield, 1);
                assertIsEvent(yield.messages()[0], *expect_event);
            }
        }

        void checkUpdate(Settings* settings, Event& control,
                uint32_t expect_id, uint8_t expect_command, uint8_t expect_value,
                Frame& state_71E_10, Frame& state_71E_21,
                Frame& state_71E_22, Event& expect_event) {
            checkUpdate(settings, control, expect_id, expect_command, expect_value,
                    &state_71E_10, &state_71E_21, &state_71E_22, nullptr, &expect_event);
        }

        void checkUpdate(Settings* settings, Event& control,
                uint32_t expect_id, uint8_t expect_command, uint8_t expect_value,
                Frame& state_71F_05, Event& expect_event) {
            checkUpdate(settings, control, expect_id, expect_command, expect_value,
                    nullptr, nullptr, nullptr, &state_71F_05, &expect_event);
        }

        void checkNoop(Settings* settings, Event& control) {
            FakeYield yield;
            settings->handle(MessageView(&control), yield);
            settings->emit(yield);
            assertSize(yield, 0);
        }
};

testF(SettingsTest, Init) {
    FakeYield yield;
    Frame frameE;
    Frame frameF;

    // Trigger settings initialization.
    Settings settings(true, &clock);
    settings.emit(yield);

    // Receive enter frames.
    fillEnterRequest(&frameE, 0x71E);
    fillEnterRequest(&frameF, 0x71F);
    assertSize(yield, 2);
    assertIsCANFrame(yield.messages()[0], frameE);
    assertIsCANFrame(yield.messages()[1], frameF);
    yield.clear();

    // Simulate response.
    fillEnterResponse(&frameE, 0x72E);
    settings.handle(MessageView(&frameE), yield);
    fillEnterResponse(&frameF, 0x72F);
    settings.handle(MessageView(&frameF), yield);
    yield.clear();

    // Receive next init frames.
    settings.emit(yield);
    fillFrame(&frameE, 0x71E, {0x02, 0x3B, 0x00});
    fillFrame(&frameF, 0x71F, {0x02, 0x3B, 0x00});
    assertSize(yield, 2);
    assertIsCANFrame(yield.messages()[0], frameE);
    assertIsCANFrame(yield.messages()[1], frameF);
    yield.clear();

    // Simulate response.
    fillFrame(&frameE, 0x72E, {0x06, 0x7B, 0x00, 0x60, 0x01, 0x0E, 0x07});
    settings.handle(MessageView(&frameE), yield);
    fillFrame(&frameF, 0x72F, {0x06, 0x7B, 0x00, 0x60, 0x01, 0x0E, 0x07});
    settings.handle(MessageView(&frameF), yield);
    yield.clear();

    // Receive next init frames. Final F frame.
    settings.emit(yield);
    fillFrame(&frameE, 0x71E, {0x02, 0x3B, 0x20});
    fillExitRequest(&frameF, 0x71F);
    assertSize(yield, 2);
    assertIsCANFrame(yield.messages()[0], frameE);
    assertIsCANFrame(yield.messages()[1], frameF);
    yield.clear();

    // Simulate response.
    fillFrame(&frameE, 0x72E, {0x06, 0x7B, 0x20, 0xC2, 0x6F, 0x73, 0xD3});
    settings.handle(MessageView(&frameE), yield);
    fillExitResponse(&frameF, 0x72F);
    settings.handle(MessageView(&frameF), yield);
    yield.clear();

    // Receive next E init Frame.
    settings.emit(yield);
    fillFrame(&frameE, 0x71E, {0x02, 0x3B, 0x40});
    assertSize(yield, 1);
    assertIsCANFrame(yield.messages()[0], frameE);
    yield.clear();

    // Simulate response.
    fillFrame(&frameE, 0x72E, {0x06, 0x7B, 0x40, 0xC2, 0xA1, 0x90, 0x01});
    settings.handle(MessageView(&frameE), yield);
    yield.clear();

    // Receive next E init Frame.
    settings.emit(yield);
    fillFrame(&frameE, 0x71E, {0x02, 0x3B, 0x60});
    assertSize(yield, 1);
    assertIsCANFrame(yield.messages()[0], frameE);
    yield.clear();

    // Simulate response.
    fillFrame(&frameE, 0x72E, {0x06, 0x7B, 0x60, 0x00, 0xFF, 0xF1, 0x70});
    settings.handle(MessageView(&frameE), yield);
    yield.clear();

    // Receive final E init frame.
    settings.emit(yield);
    fillExitRequest(&frameE, 0x71E);
    assertSize(yield, 1);
    assertIsCANFrame(yield.messages()[0], frameE);
    yield.clear();

    // Simulate response.
    fillExitResponse(&frameE, 0x72E);
    settings.handle(MessageView(&frameE), yield);
    yield.clear();
}

testF(SettingsTest, RequestState) {
    FakeYield yield;
    Settings settings(false, &clock);

    Frame frameE;
    Frame frameF;

    // Send control event to trigger retrieve.
    RequestCommand command(SubSystem::SETTINGS, (uint8_t)SettingsEvent::STATE);
    settings.handle(MessageView(&command), yield);
    yield.clear();

    // Receive enter request frames.
    settings.emit(yield);
    fillEnterRequest(&frameE, 0x71E);
    fillEnterRequest(&frameF, 0x71F);
    assertSize(yield, 2);
    assertIsCANFrame(yield.messages()[0], frameE);
    assertIsCANFrame(yield.messages()[1], frameF);
    yield.clear();

    // Simulate response.
    fillEnterResponse(&frameE, 0x72E);
    settings.handle(MessageView(&frameE), yield);
    fillEnterResponse(&frameF, 0x72F);
    settings.handle(MessageView(&frameF), yield);
    yield.clear();

    // Receive initial state frames.
    settings.emit(yield);
    fillState0221Request(&frameE, 0x71E);
    fillState0221Request(&frameF, 0x71F);
    assertSize(yield, 2);
    assertIsCANFrame(yield.messages()[0], frameE);
    assertIsCANFrame(yield.messages()[1], frameF);
    yield.clear();

    // Simulate response.
    fillFrame(&frameE, 0x72E, {0x10, 0x11, 0x61, 0x01, 0x00, 0x1E, 0x24, 0x00});
    settings.handle(MessageView(&frameE), yield);
    fillFrame(&frameF, 0x72F, {0x05, 0x61, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF});
    settings.handle(MessageView(&frameF), yield);
    yield.clear();

    // Receive next E state frame and F exit frame.
    settings.emit(yield);
    fillState3000Request(&frameE, 0x71E);
    fillExitRequest(&frameF, 0x71F);
    assertSize(yield, 2);
    assertIsCANFrame(yield.messages()[0], frameE);
    assertIsCANFrame(yield.messages()[1], frameF);
    yield.clear();

    // Simulate response.
    fillFrame(&frameE, 0x72E, {0x21, 0x10, 0x0C, 0x40, 0x40, 0x01, 0x64, 0x00});
    settings.handle(MessageView(&frameE), yield);
    fillFrame(&frameE, 0x72E, {0x22, 0x94, 0x00, 0x00, 0x47, 0xFF, 0xFF, 0xFF});
    settings.handle(MessageView(&frameE), yield);
    fillExitResponse(&frameF, 0x72F);
    settings.handle(MessageView(&frameF), yield);
    yield.clear();

    // Receive E exit frame.
    settings.emit(yield);
    fillExitRequest(&frameE, 0x71E);
    assertSize(yield, 1);
    assertIsCANFrame(yield.messages()[0], frameE);
    yield.clear();

    // Simulate response.
    fillExitResponse(&frameE, 0x72E);
    settings.handle(MessageView(&frameE), yield);
    yield.clear();

    // Ensure settings are at default.
    Event event(SubSystem::SETTINGS, (uint8_t)SettingsEvent::STATE, {0x00, 0x00, 0x00, 0x00});
    settings.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], event);
}

testF(SettingsTest, FactoryReset) {
    FakeYield yield;
    Settings settings(false, &clock);

    Frame frameE;
    Frame frameF;

    // Send control event to trigger reset.
    Event event((uint8_t)SubSystem::SETTINGS, (uint8_t)SettingsEvent::FACTORY_RESET_CMD);
    settings.handle(MessageView(&event), yield);
    yield.clear();

    // Receive enter request frames.
    settings.emit(yield);
    fillEnterRequest(&frameE, 0x71E);
    fillEnterRequest(&frameF, 0x71F);
    assertSize(yield, 2);
    assertIsCANFrame(yield.messages()[0], frameE);
    assertIsCANFrame(yield.messages()[1], frameF);
    yield.clear();

    // Simulate response.
    fillEnterResponse(&frameE, 0x72E);
    settings.handle(MessageView(&frameE), yield);
    fillEnterResponse(&frameF, 0x72F);
    settings.handle(MessageView(&frameF), yield);
    yield.clear();

    // Send reset all command.
    settings.emit(yield);
    fillResetRequest(&frameE, 0x71E);
    fillResetRequest(&frameF, 0x71F);
    assertSize(yield, 2);
    assertIsCANFrame(yield.messages()[0], frameE);
    assertIsCANFrame(yield.messages()[1], frameF);
    yield.clear();

    // Simulate response.
    fillResetResponse(&frameE, 0x72E);
    settings.handle(MessageView(&frameE), yield);
    fillResetResponse(&frameF, 0x72F);
    settings.handle(MessageView(&frameF), yield);
    yield.clear();

    // Receive initial state frames.
    settings.emit(yield);
    fillState0221Request(&frameE, 0x71E);
    fillState0221Request(&frameF, 0x71F);
    assertSize(yield, 2);
    assertIsCANFrame(yield.messages()[0], frameE);
    assertIsCANFrame(yield.messages()[1], frameF);
    yield.clear();

    // Simulate response.
    fillFrame(&frameE, 0x72E, {0x10, 0x11, 0x61, 0x01, 0x00, 0x1E, 0x24, 0x00});
    settings.handle(MessageView(&frameE), yield);
    fillFrame(&frameF, 0x72F, {0x05, 0x61, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF});
    settings.handle(MessageView(&frameF), yield);

    // Receive next E state frame and F exit frame.
    settings.emit(yield);
    fillState3000Request(&frameE, 0x71E);
    fillExitRequest(&frameF, 0x71F);
    assertSize(yield, 2);
    assertIsCANFrame(yield.messages()[0], frameE);
    assertIsCANFrame(yield.messages()[1], frameF);
    yield.clear();

    // Simulate response.
    fillFrame(&frameE, 0x72E, {0x21, 0x10, 0x0C, 0x40, 0x40, 0x01, 0x64, 0x00});
    settings.handle(MessageView(&frameE), yield);
    fillFrame(&frameE, 0x72E, {0x22, 0x94, 0x00, 0x00, 0x47, 0xFF, 0xFF, 0xFF});
    settings.handle(MessageView(&frameE), yield);
    fillExitResponse(&frameF, 0x72F);
    settings.handle(MessageView(&frameF), yield);
    yield.clear();

    // Receive E exit frame.
    settings.emit(yield);
    fillExitRequest(&frameE, 0x71E);
    assertSize(yield, 1);
    assertIsCANFrame(yield.messages()[0], frameE);
    yield.clear();

    // Simulate response.
    fillExitResponse(&frameE, 0x72E);
    settings.handle(MessageView(&frameE), yield);
    yield.clear();

    // Ensure settings are at default.
    event = Event((uint8_t)SubSystem::SETTINGS, (uint8_t)SettingsEvent::STATE, {0x00, 0x00, 0x00, 0x00});
    settings.emit(yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], event);
}

testF(SettingsTest, ToggleAutoInteriorIllumination) {
    Settings settings(false, &clock);

    // Initial state.
    Event control((uint8_t)SubSystem::SETTINGS, (uint8_t)SettingsEvent::TOGGLE_AUTO_INTERIOR_ILLUM_CMD);
    Event expect((uint8_t)SubSystem::SETTINGS, (uint8_t)SettingsEvent::STATE, {0x00, 0x00, 0x00, 0x00});
    Frame state10 = {0x72E, 8, {0x10, 0x11, 0x61, 0x01, 0x00, 0x1E, 0x24, 0x00}};
    Frame state21 = {0x72E, 8, {0x21, 0x10, 0x0C, 0x40, 0x40, 0x01, 0x64, 0x00}};
    Frame state22 = {0x72E, 8, {0x22, 0x94, 0x00, 0x00, 0x47, 0xFF, 0xFF, 0xFF}};

    // Toggle setting on.
    flipBit(expect.data, 0, 0);
    setBit(state10.data(), 4, 5, 1);
    checkUpdate(&settings, control, 0x71E, 0x10, 0x01, state10, state21, state22, expect);

    // Toggle setting off.
    flipBit(expect.data, 0, 0);
    setBit(state10.data(), 4, 5, 0);
    checkUpdate(&settings, control, 0x71E, 0x10, 0x00, state10, state21, state22, expect);

    // Toggle setting on.
    flipBit(expect.data, 0, 0);
    setBit(state10.data(), 4, 5, 1);
    checkUpdate(&settings, control, 0x71E, 0x10, 0x01, state10, state21, state22, expect);
}

testF(SettingsTest, ToggleSlideDriverSeat) {
    Settings settings(false, &clock);

    // Initial state.
    Event control((uint8_t)SubSystem::SETTINGS, (uint8_t)SettingsEvent::TOGGLE_SLIDE_DRIVER_SEAT_CMD);
    Event expect((uint8_t)SubSystem::SETTINGS, (uint8_t)SettingsEvent::STATE, {0x00, 0x00, 0x00, 0x00});
    Frame state05 = {0x72F, 8, {0x05, 0x61, 0x01, 0x00, 0x00, 0x00, 0xFF, 0xFF}};

    // Toggle setting on.
    flipBit(expect.data, 0, 1);
    setBit(state05.data(), 3, 0, 1);
    checkUpdate(&settings, control, 0x71F, 0x01, 0x01, state05, expect);

    // Toggle setting off.
    flipBit(expect.data, 0, 1);
    setBit(state05.data(), 3, 0, 0);
    checkUpdate(&settings, control, 0x71F, 0x01, 0x00, state05, expect);

    // Toggle setting on.
    flipBit(expect.data, 0, 1);
    setBit(state05.data(), 3, 0, 1);
    checkUpdate(&settings, control, 0x71F, 0x01, 0x01, state05, expect);
}

testF(SettingsTest, ToggleSpeedSensingWipers) {
    Settings settings(false, &clock);

    // Initial state.
    Event control((uint8_t)SubSystem::SETTINGS, (uint8_t)SettingsEvent::TOGGLE_SPEED_SENSING_WIPER_CMD);
    Event expect((uint8_t)SubSystem::SETTINGS, (uint8_t)SettingsEvent::STATE, {0x00, 0x00, 0x00, 0x00});
    Frame state10 = {0x72E, 8, {0x10, 0x11, 0x61, 0x01, 0x00, 0x1E, 0x24, 0x00}};
    Frame state21 = {0x72E, 8, {0x21, 0x10, 0x0C, 0x40, 0x40, 0x01, 0x64, 0x00}};
    Frame state22 = {0x72E, 8, {0x22, 0x94, 0x00, 0x00, 0x47, 0xFF, 0xFF, 0xFF}};

    // Toggle setting on.
    flipBit(expect.data, 0, 2);
    setBit(state22.data(), 1, 7, 0);
    checkUpdate(&settings, control, 0x71E, 0x47, 0x00, state10, state21, state22, expect);

    // Toggle setting off.
    flipBit(expect.data, 0, 2);
    setBit(state22.data(), 1, 7, 1);
    checkUpdate(&settings, control, 0x71E, 0x47, 0x01, state10, state21, state22, expect);

    // Toggle setting on.
    flipBit(expect.data, 0, 2);
    setBit(state22.data(), 1, 7, 0);
    checkUpdate(&settings, control, 0x71E, 0x47, 0x00, state10, state21, state22, expect);
}

testF(SettingsTest, AutoHeadlightSensitivity) {
    Settings settings(false, &clock);

    // Initial state.
    Event control((uint8_t)SubSystem::SETTINGS, (uint8_t)SettingsEvent::NEXT_AUTO_HEADLIGHT_SENS_CMD);
    Event expect((uint8_t)SubSystem::SETTINGS, (uint8_t)SettingsEvent::STATE, {0x00, 0x00, 0x00, 0x00});
    Frame state10 = {0x72E, 8, {0x10, 0x11, 0x61, 0x01, 0x00, 0x1E, 0x24, 0x00}};
    Frame state21 = {0x72E, 8, {0x21, 0x10, 0x0C, 0x40, 0x40, 0x01, 0x64, 0x00}};
    Frame state22 = {0x72E, 8, {0x22, 0x94, 0x00, 0x00, 0x47, 0xFF, 0xFF, 0xFF}};

    // Increase setting.
    expect.data[1] = 1;
    state21.data()[2] = 0x00;
    checkUpdate(&settings, control, 0x71E, 0x37, 0x00, state10, state21, state22, expect);

    // Increase setting.
    expect.data[1] = 2;
    state21.data()[2] = (0x01 << 2);
    checkUpdate(&settings, control, 0x71E, 0x37, 0x01, state10, state21, state22, expect);

    // Increase setting.
    expect.data[1] = 3;
    state21.data()[2] = (0x02 << 2);
    checkUpdate(&settings, control, 0x71E, 0x37, 0x02, state10, state21, state22, expect);

    // Increase setting when at max.
    checkNoop(&settings, control);

    // Decrease setting.
    control.id = (uint8_t)SettingsEvent::PREV_AUTO_HEADLIGHT_SENS_CMD;
    expect.data[1] = 2;
    state21.data()[2] = (0x01 << 2);
    checkUpdate(&settings, control, 0x71E, 0x37, 0x01, state10, state21, state22, expect);

    // Decrease setting.
    expect.data[1] = 1;
    state21.data()[2] = 0x00;
    checkUpdate(&settings, control, 0x71E, 0x37, 0x00, state10, state21, state22, expect);

    // Decrease setting.
    expect.data[1] = 0;
    state21.data()[2] = (0x03 << 2);
    checkUpdate(&settings, control, 0x71E, 0x37, 0x03, state10, state21, state22, expect);

    // Decrease setting when at min.
    checkNoop(&settings, control);
}

testF(SettingsTest, AutoHeadlightOffDelay) {
    Settings settings(false, &clock);

    // Initial state.
    byte value = 0x00;
    Event control((uint8_t)SubSystem::SETTINGS, (uint8_t)SettingsEvent::NEXT_AUTO_HEADLIGHT_OFF_DELAY_CMD);
    Event expect((uint8_t)SubSystem::SETTINGS, (uint8_t)SettingsEvent::STATE, {0x00, 0x00, 0x00, 0x00});
    Frame state10 = {0x72E, 8, {0x10, 0x11, 0x61, 0x01, 0x00, 0x1E, 0x24, 0x00}};
    Frame state21 = {0x72E, 8, {0x21, 0x10, 0x0C, 0x40, 0x40, 0x01, 0x64, 0x00}};
    Frame state22 = {0x72E, 8, {0x22, 0x94, 0x00, 0x00, 0x47, 0xFF, 0xFF, 0xFF}};

    // Increase setting to 30s.
    value = 0x02;
    expect.data[1] = 0x01 | (2 << 4);
    setAutoHeadlightDelayState(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22, expect);

    // Increase setting to 45s (default).
    value = 0x00;
    expect.data[1] = 0x01 | (3 << 4);
    setAutoHeadlightDelayState(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22, expect);

    // Increase setting to 60s.
    value = 0x03;
    expect.data[1] = 0x01 | (4 << 4);
    setAutoHeadlightDelayState(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22, expect);

    // Increase setting to 90s.
    value = 0x04;
    expect.data[1] = 0x01 | (6 << 4);
    setAutoHeadlightDelayState(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22, expect);

    // Increase setting to 120s.
    value = 0x05;
    expect.data[1] = 0x01 | (8 << 4);
    setAutoHeadlightDelayState(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22, expect);

    // Increase setting to 150s.
    value = 0x06;
    expect.data[1] = 0x01 | (10 << 4);
    setAutoHeadlightDelayState(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22, expect);

    // Increase setting to 180s.
    value = 0x07;
    expect.data[1] = 0x01 | (12 << 4);
    setAutoHeadlightDelayState(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22, expect);

    // Increase setting when at max.
    checkNoop(&settings, control);

    // Decrease setting to 150s.
    control.id = (uint8_t)SettingsEvent::PREV_AUTO_HEADLIGHT_OFF_DELAY_CMD;
    value = 0x06;
    expect.data[1] = 0x01 | (10 << 4);
    setAutoHeadlightDelayState(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22, expect);

    // Decrease setting to 120s.
    value = 0x05;
    expect.data[1] = 0x01 | (8 << 4);
    setAutoHeadlightDelayState(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22, expect);

    // Decrease setting to 90s.
    value = 0x04;
    expect.data[1] = 0x01 | (6 << 4);
    setAutoHeadlightDelayState(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22, expect);

    // Decrease setting to 60s.
    value = 0x03;
    expect.data[1] = 0x01 | (4 << 4);
    setAutoHeadlightDelayState(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22, expect);

    // Decrease setting to 45s (default).
    value = 0x00;
    expect.data[1] = 0x01 | (3 << 4);
    setAutoHeadlightDelayState(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22, expect);

    // Decrease setting to 30s.
    value = 0x02;
    expect.data[1] = 0x01 | (2 << 4);
    setAutoHeadlightDelayState(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22, expect);

    // Decrease setting to 0s.
    value = 0x01;
    expect.data[1] = 0x01 | (0 << 4);
    setAutoHeadlightDelayState(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x39, value, state10, state21, state22, expect);

    // Decrease setting when at min.
    checkNoop(&settings, control);
}

testF(SettingsTest, ToggleSelectiveDoorUnlock) {
    Settings settings(false, &clock);

    // Initial state.
    Event control((uint8_t)SubSystem::SETTINGS, (uint8_t)SettingsEvent::TOGGLE_SELECTIVE_DOOR_UNLOCK_CMD);
    Event expect((uint8_t)SubSystem::SETTINGS, (uint8_t)SettingsEvent::STATE, {0x00, 0x00, 0x00, 0x00});
    Frame state10 = {0x72E, 8, {0x10, 0x11, 0x61, 0x01, 0x00, 0x1E, 0x24, 0x00}};
    Frame state21 = {0x72E, 8, {0x21, 0x10, 0x0C, 0x40, 0x40, 0x01, 0x64, 0x00}};
    Frame state22 = {0x72E, 8, {0x22, 0x94, 0x00, 0x00, 0x47, 0xFF, 0xFF, 0xFF}};

    // Toggle setting on.
    flipBit(expect.data, 2, 0);
    setBit(state10.data(), 4, 7, 1);
    checkUpdate(&settings, control, 0x71E, 0x02, 0x01, state10, state21, state22, expect);

    // Toggle setting off.
    flipBit(expect.data, 2, 0);
    setBit(state10.data(), 4, 7, 0);
    checkUpdate(&settings, control, 0x71E, 0x02, 0x00, state10, state21, state22, expect);

    // Toggle setting on.
    flipBit(expect.data, 2, 0);
    setBit(state10.data(), 4, 7, 1);
    checkUpdate(&settings, control, 0x71E, 0x02, 0x01, state10, state21, state22, expect);
}

testF(SettingsTest, AutoReLockTime) {
    Settings settings(false, &clock);

    // Initial state.
    byte value = 0x00;
    Event control((uint8_t)SubSystem::SETTINGS, (uint8_t)SettingsEvent::NEXT_AUTO_RELOCK_TIME_CMD);
    Event expect((uint8_t)SubSystem::SETTINGS, (uint8_t)SettingsEvent::STATE, {0x00, 0x00, 0x00, 0x00});
    Frame state10 = {0x72E, 8, {0x10, 0x11, 0x61, 0x01, 0x00, 0x1E, 0x24, 0x00}};
    Frame state21 = {0x72E, 8, {0x21, 0x10, 0x0C, 0x40, 0x40, 0x01, 0x64, 0x00}};
    Frame state22 = {0x72E, 8, {0x22, 0x94, 0x00, 0x00, 0x47, 0xFF, 0xFF, 0xFF}};

    // Increase setting to 1m (default).
    value = 0x00;
    expect.data[2] = 1 << 4;
    setAutoReLockTime(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x2F, value, state10, state21, state22, expect);

    // Increase setting to 5m.
    value = 0x02;
    expect.data[2] = 5 << 4;
    setAutoReLockTime(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x2F, value, state10, state21, state22, expect);

    // Increase setting when at max.
    checkNoop(&settings, control);

    // Decrease setting to 1m (default).
    control.id = (uint8_t)SettingsEvent::PREV_AUTO_RELOCK_TIME_CMD;
    value = 0x00;
    expect.data[2] = 1 << 4;
    setAutoReLockTime(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x2F, value, state10, state21, state22, expect);

    // Decrease setting to off.
    value = 0x01;
    expect.data[2] = 0 << 4;
    setAutoReLockTime(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x2F, value, state10, state21, state22, expect);

    // Decrease setting when at min.
    checkNoop(&settings, control);
}

testF(SettingsTest, RemoteKeyResponseHorn) {
    Settings settings(false, &clock);

    // Initial state.
    Event control((uint8_t)SubSystem::SETTINGS, (uint8_t)SettingsEvent::TOGGLE_REMOTE_KEY_RESP_HORN_CMD);
    Event expect((uint8_t)SubSystem::SETTINGS, (uint8_t)SettingsEvent::STATE, {0x00, 0x00, 0x00, 0x00});
    Frame state10 = {0x72E, 8, {0x10, 0x11, 0x61, 0x01, 0x00, 0x1E, 0x24, 0x00}};
    Frame state21 = {0x72E, 8, {0x21, 0x10, 0x0C, 0x40, 0x40, 0x01, 0x64, 0x00}};
    Frame state22 = {0x72E, 8, {0x22, 0x94, 0x00, 0x00, 0x47, 0xFF, 0xFF, 0xFF}};

    // Toggle setting on.
    flipBit(expect.data, 3, 0);
    setBit(state10.data(), 7, 3, 1);
    checkUpdate(&settings, control, 0x71E, 0x2A, 0x01, state10, state21, state22, expect);

    // Toggle setting off.
    flipBit(expect.data, 3, 0);
    setBit(state10.data(), 7, 3, 0);
    checkUpdate(&settings, control, 0x71E, 0x2A, 0x00, state10, state21, state22, expect);

    // Toggle setting on.
    flipBit(expect.data, 3, 0);
    setBit(state10.data(), 7, 3, 1);
    checkUpdate(&settings, control, 0x71E, 0x2A, 0x01, state10, state21, state22, expect);
}

testF(SettingsTest, RemoteKeyResponseLights) {
    Settings settings(false, &clock);

    // Initial state.
    byte value = 0x00;
    Event control((uint8_t)SubSystem::SETTINGS, (uint8_t)SettingsEvent::NEXT_REMOTE_KEY_RESP_LIGHTS_CMD);
    Event expect((uint8_t)SubSystem::SETTINGS, (uint8_t)SettingsEvent::STATE, {0x00, 0x00, 0x00, 0x00});
    Frame state10 = {0x72E, 8, {0x10, 0x11, 0x61, 0x01, 0x00, 0x1E, 0x24, 0x00}};
    Frame state21 = {0x72E, 8, {0x21, 0x10, 0x0C, 0x40, 0x40, 0x01, 0x64, 0x00}};
    Frame state22 = {0x72E, 8, {0x22, 0x94, 0x00, 0x00, 0x47, 0xFF, 0xFF, 0xFF}};

    // Increase setting to "unlock".
    value = 0x01;
    expect.data[3] = 1 << 2;
    setRemoteKeyResponseLights(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x2E, value, state10, state21, state22, expect);

    // Increase setting to "lock".
    value = 0x02;
    expect.data[3] = 2 << 2;
    setRemoteKeyResponseLights(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x2E, value, state10, state21, state22, expect);

    // Increase setting to "on".
    value = 0x03;
    expect.data[3] = 3 << 2;
    setRemoteKeyResponseLights(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x2E, value, state10, state21, state22, expect);

    // Increase setting when at max.
    checkNoop(&settings, control);

    // Decrease setting to "lock".
    control.id = (uint8_t)SettingsEvent::PREV_REMOTE_KEY_RESP_LIGHTS_CMD;
    value = 0x02;
    expect.data[3] = 2 << 2;
    setRemoteKeyResponseLights(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x2E, value, state10, state21, state22, expect);

    // Decrease setting to "unlock".
    value = 0x01;
    expect.data[3] = 1 << 2;
    setRemoteKeyResponseLights(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x2E, value, state10, state21, state22, expect);

    // Decrease setting to "off".
    value = 0x00;
    expect.data[3] = 0 << 2;
    setRemoteKeyResponseLights(&state21, value);
    checkUpdate(&settings, control, 0x71E, 0x2E, value, state10, state21, state22, expect);

    // Decrease setting when at min.
    checkNoop(&settings, control);
}

}  // namespace R51

// Test boilerplate.
void setup() {
#ifdef ARDUINO
    delay(1000);
#endif
    SERIAL_PORT_MONITOR.begin(115200);
    while(!SERIAL_PORT_MONITOR);
}

void loop() {
    aunit::TestRunner::run();
    delay(1);
}
