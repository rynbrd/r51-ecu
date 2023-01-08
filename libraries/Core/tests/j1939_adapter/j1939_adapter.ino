#include <AUnit.h>
#include <Arduino.h>
#include <Canny.h>
#include <Core.h>
#include <Test.h>

namespace R51 {

using namespace aunit;
using ::Canny::J1939Message;

class J1939FilteredAdapter : public J1939Adapter {
    public:
        uint8_t route(const Event& event) override {
            if (event.subsystem == 0xEE) {
                return 0xEE;
            } else if (event.subsystem == 0xEF) {
                return Canny::NullAddress;
            }
            return 0xFF;
        }

        bool readFilter(const Event& event) override {
            return event.id != 0x10;
        }
};

test(J1939AdapterTest, WriteEvent) {
    FakeYield yield;
    J1939FilteredAdapter adapter;
    J1939Claim claim(0xAA, 0);

    Event event(0x01, 0x01, (uint8_t[]){0x11, 0x22, 0x33, 0x44, 0x55, 0x66});
    J1939Message expect(0xFF00, 0xAA, 0xFF);
    expect.data({0x01, 0x01, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66});

    adapter.handle(MessageView(&claim), yield);
    adapter.handle(MessageView(&event), yield);
    assertSize(yield, 1);
    assertIsJ1939Message(yield.messages()[0], expect);
}

test(J1939AdapterTest, ReadBroadcastEvent) {
    FakeYield yield;
    J1939FilteredAdapter adapter;
    J1939Claim claim(0xAA, 0);

    J1939Message msg(0xFF00, 0x21);
    msg.data({0x01, 0x01, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66});
    Event expect(0x01, 0x01, (uint8_t[]){0x11, 0x22, 0x33, 0x44, 0x55, 0x66});

    adapter.handle(MessageView(&claim), yield);
    adapter.handle(MessageView(&msg), yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
}

test(J1939AdapterTest, ReadAddressedEvent) {
    FakeYield yield;
    J1939FilteredAdapter adapter;
    J1939Claim claim(0xAA, 0);

    J1939Message msg(0xEF00, 0x21, 0xAA);
    msg.data({0x01, 0x01, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66});
    Event expect(0x01, 0x01, (uint8_t[]){0x11, 0x22, 0x33, 0x44, 0x55, 0x66});

    adapter.handle(MessageView(&claim), yield);
    adapter.handle(MessageView(&msg), yield);
    assertSize(yield, 1);
    assertIsEvent(yield.messages()[0], expect);
}

test(J1939AdapterTest, ReadFilteredEvent) {
    FakeYield yield;
    J1939FilteredAdapter adapter;
    J1939Claim claim(0xAA, 0);

    J1939Message msg(0xEF00, 0x21, 0xAA);
    msg.data({0x01, 0x10, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66});

    adapter.handle(MessageView(&claim), yield);
    adapter.handle(MessageView(&msg), yield);
    assertSize(yield, 0);
}

test(J1939AdapterTest, IgnoreMessageToOtherDest) {
    FakeYield yield;
    J1939FilteredAdapter adapter;
    J1939Claim claim(0xAA, 0);

    J1939Message msg(0xEF00, 0x21, 0xAB);
    msg.data({0x01, 0x01, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66});

    adapter.handle(MessageView(&claim), yield);
    adapter.handle(MessageView(&msg), yield);
    assertSize(yield, 0);
}

test(J1939AdapterTest, IgnoreMessageFromSelf) {
    FakeYield yield;
    J1939FilteredAdapter adapter;
    J1939Claim claim(0xAA, 0);

    J1939Message msg(0xFF00, 0xAA);
    msg.data({0x01, 0x01, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66});

    adapter.handle(MessageView(&claim), yield);
    adapter.handle(MessageView(&msg), yield);
    assertSize(yield, 0);
}

test(J1939AdapterTest, RouteEvent) {
    FakeYield yield;
    J1939FilteredAdapter adapter;
    J1939Claim claim(0xAA, 0);

    Event event(0xEE, 0x01, (uint8_t[]){0x11, 0x22, 0x33, 0x44, 0x55, 0x66});
    J1939Message expect(0xEF00, 0xAA, 0xEE);
    expect.data({0xEE, 0x01, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66});

    adapter.handle(MessageView(&claim), yield);
    adapter.handle(MessageView(&event), yield);
    assertSize(yield, 1);
    assertIsJ1939Message(yield.messages()[0], expect);
}

test(J1939AdapterTest, RouteNullEvent) {
    FakeYield yield;
    J1939FilteredAdapter adapter;
    J1939Claim claim(0xAA, 0);

    Event event(0xEF, 0x01, (uint8_t[]){0x11, 0x22, 0x33, 0x44, 0x55, 0x66});

    adapter.handle(MessageView(&claim), yield);
    adapter.handle(MessageView(&event), yield);
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
