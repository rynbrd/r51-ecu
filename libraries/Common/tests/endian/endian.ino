#include <AUnit.h>
#include <Arduino.h>
#include <Canny.h>
#include <Common.h>

namespace R51 {

using namespace aunit;
using ::Canny::Frame;
using ::Canny::J1939Message;

test(EndianTest, UInt32ToNetwork) {
    uint32_t src = 0x11223344;
    uint8_t dest[4];
    uint8_t expect[] = {0x11, 0x22, 0x33, 0x44};
    UInt32ToNetwork(dest, src);
    assertTrue(memcmp(expect, dest, 4) == 0);
}

test(EndianTest, NetworkToUnit32) {
    uint8_t src[] = {0x11, 0x22, 0x33, 0x44};
    uint32_t dest;
    uint32_t expect = 0x11223344;
    NetworkToUInt32(&dest, src);
    assertEqual(expect, dest);
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
