#include <AUnit.h>
#include <Arduino.h>
#include <Canny.h>
#include <Controls.h>
#include <Core.h>
#include <Faker.h>
#include <Test.h>

namespace R51 {

using namespace aunit;
using ::Canny::J1939Message;
using ::Faker::FakeClock;

class FusionTest : public TestOnce {
    public:
        void start(Fusion* fusion) {
            // Controller claims an addres.
            J1939Claim claim(0x19, 0);
            fusion->handle(MessageView(&claim), yield);
            yield.clear();

            // Stereo discovery.
            J1939Message msg(0x1F014, 0x0A, 0xFF, 0x06);
            msg.data({0xA0, 0x86, 0x35, 0x08, 0x8E, 0x12, 0x4D, 0x53});
            fusion->handle(MessageView(&msg), yield);
            yield.clear();

            // Power on.
            Event event(SubSystem::AUDIO, (uint8_t)AudioEvent::POWER_ON_CMD);
            fusion->handle(MessageView(&event), yield);
            yield.clear();
            clock.delay(5001);
            msg.id(0x1DFF040A);
            msg.data({0x60, 0x05, 0xA3, 0x99, 0x20, 0x80, 0x01, 0xFF});
            fusion->handle(MessageView(&msg), yield);
            yield.clear();
            fusion->emit(yield);
            yield.clear();
        }

        FakeClock clock;
        FakeYield yield;

        static const uint8_t addr = 0x19;
        static const uint8_t hu_addr = 0x0A;
};

testF(FusionTest, StateDetectionDoesntCrash) {
    Fusion f(&clock);
    start(&f);

    // this crashes, because the messages from Fusion cause the moduel to
    // generate event messages that pass the detectState checks, causing a
    // feedback loop
    // 1DFF040A#20:0D:A3:99:0B:80:00:01
    // 1DFF040A#21:B0:C4:12:00:9B:00:00
    // 1CFF0019#21:08:00:00:12:C4:B0:FF
    // 1CFF0019#21:08:00:12:00:00:08:FF

    J1939Message msg0, msg1, msg2, msg3;
    msg0.id(0x1DFF040A);
    msg0.data({0x20, 0x0D, 0xA3, 0x99, 0x0B, 0x80, 0x00, 0x01});
    msg1.id(0x1DFF040A);
    msg1.data({0x21, 0xB0, 0xC4, 0x12, 0x00, 0x9B, 0x00, 0x00});
    msg2.id(0x1CFF0019);
    msg2.data({0x21, 0x08, 0x00, 0x00, 0x12, 0xC4, 0xB0, 0xFF});
    msg3.id(0x1CFF0019);
    msg3.data({0x21, 0x08, 0x00, 0x12, 0x00, 0x00, 0x08, 0xFF});

    f.handle(MessageView(&msg0), yield);
    assertSize(yield, 0);

    f.handle(MessageView(&msg1), yield);
    assertSize(yield, 1);
    yield.clear();

    f.handle(MessageView(&msg2), yield);
    assertSize(yield, 0);

    f.handle(MessageView(&msg3), yield);
    assertSize(yield, 0);
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
