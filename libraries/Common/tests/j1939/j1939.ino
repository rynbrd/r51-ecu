#include <AUnit.h>
#include <Arduino.h>
#include <Canny.h>
#include <Common.h>
#include <Test.h>

namespace R51 {

using namespace aunit;
using ::Canny::Frame;
using ::Canny::J1939Message;

test(J1939AddressClaimTest, Init) {
    // Should emit an address claim with the preferred address on first emit.
    FakeYield yield;
    uint8_t address = 0x0A;
    uint64_t name = 0x000013B000FFFAC0;
    J1939AddressClaim node(address, name);


    Frame expect_frame(0x18EEFF0A, 1, {0x00, 0x00, 0x13, 0xB0, 0x00, 0xFF, 0xFA, 0xC0});
    J1939Claim expect_claim(address, name);

    node.emit(yield);
    assertSize(yield, 2);
    assertIsJ1939Message(yield.messages()[0], expect_frame);
    assertIsJ1939Claim(yield.messages()[1], expect_claim);
}

test(J1939AddressClaimTest, Request) {
    FakeYield yield;
    uint8_t address = 0x0A;
    uint64_t name = 0x000013B000FFFAC0;
    J1939AddressClaim node(address, name);

    // send initial claim
    node.emit(yield);
    yield.clear();

    // handle request address claim
    J1939Message msg(0xEA00, 0x21, 0xFF, 0x06);
    msg.data({0x00, 0xEE, 0x00});
    node.handle(msg, yield);
    assertSize(yield, 0);

    // we should respond with our address
    Frame expect_frame(0x18EEFF0A, 1, {0x00, 0x00, 0x13, 0xB0, 0x00, 0xFF, 0xFA, 0xC0});

    node.emit(yield);
    assertSize(yield, 1);
    assertIsJ1939Message(yield.messages()[0], expect_frame);
}

test(J1939AddressClaimTest, NoArbitraryAddressCannotClaim) {
    FakeYield yield;
    uint8_t address = 0x0A;
    uint64_t name = 0x000013B000FFFAC0;
    J1939AddressClaim node(address, name);

    // send initial claim
    node.emit(yield);
    yield.clear();

    // handle address claim with same address and higher priority name
    J1939Message msg(0xEA00, address, 0xFF, 0x06);
    msg.data({0x00, 0x00, 0x0B, 0xB0, 0x00, 0xFF, 0xFA, 0xC0});
    node.handle(msg, yield);
    assertSize(yield, 0);

    // we should respond with a null address
    Frame expect_frame(0x18EEFFFE, 1, {0x00, 0x00, 0x13, 0xB0, 0x00, 0xFF, 0xFA, 0xC0});
    J1939Claim expect_claim(Canny::NullAddress, name);

    node.emit(yield);
    assertSize(yield, 2);
    assertIsJ1939Message(yield.messages()[0], expect_frame);
    assertIsJ1939Claim(yield.messages()[1], expect_claim);
}

test(J1939AddressClaimTest, ArbitraryAddressClaim) {
    FakeYield yield;
    uint8_t address = 0x0A;
    uint64_t name = 0x000013B000FFFAC1;
    J1939AddressClaim node(address, name);

    // send initial claim
    node.emit(yield);
    yield.clear();

    // handle address claim with same address and higher priority name
    J1939Message msg(0xEA00, address, 0xFF, 0x06);
    msg.data({0x00, 0x00, 0x0B, 0xB0, 0x00, 0xFF, 0xFA, 0xC0});
    node.handle(msg, yield);
    assertSize(yield, 0);

    // we should response with the next address
    Frame expect_frame(0x18EEFF0B, 1, {0x00, 0x00, 0x13, 0xB0, 0x00, 0xFF, 0xFA, 0xC1});
    J1939Claim expect_claim(0x0B, name);

    node.emit(yield);
    assertSize(yield, 2);
    assertIsJ1939Message(yield.messages()[0], expect_frame);
    assertIsJ1939Claim(yield.messages()[1], expect_claim);
}

test(J1939AddressClaimTest, ArbitraryAddressCannotClaim) {
    FakeYield yield;
    uint8_t address = 0x0A;
    uint64_t name = 0x000013B000FFFAC1;
    J1939AddressClaim node(address, name);

    // send initial claim
    node.emit(yield);
    yield.clear();

    // attempt to claim all possible addresses
    for (uint8_t i = 0; i < 253; ++i) {
        uint8_t next;
        if (i == 252) {
            next = Canny::NullAddress;
        } else {
            next = address + 1;
            if (next >= 254) {
                next = 1;
            }
        }

        J1939Message msg(0xEA00, address, 0xFF, 0x06);
        msg.data({0x00, 0x00, 0x0B, 0xB0, 0x00, 0xFF, 0xFA, 0xC0});
        node.handle(msg, yield);
        assertSize(yield, 0);

        // we should response with the next address
        Frame expect_frame(0x18EEFF00 | next, 1, {0x00, 0x00, 0x13, 0xB0, 0x00, 0xFF, 0xFA, 0xC1});
        J1939Claim expect_claim(next, name);

        node.emit(yield);
        assertSize(yield, 2);
        assertIsJ1939Message(yield.messages()[0], expect_frame);
        assertIsJ1939Claim(yield.messages()[1], expect_claim);
        yield.clear();

        address = next;
    }
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
