#include <AUnit.h>
#include <Arduino.h>
#include <Blink.h>
#include <Canny.h>
#include <Common.h>
#include <Test.h>

using namespace aunit;

namespace R51 {

using ::Canny::J1939Message;

test(KeyboxTest, SetOutput) {
    FakeYield yield;
    uint8_t ecm_address = 0x0A;
    uint8_t pdm_address = 0x21;
    uint8_t pdm_id = 0x01;
    BlinkKeybox keybox(pdm_address, pdm_id);

    // assign source address
    J1939Claim claim(ecm_address, 0);
    keybox.handle(claim, yield);
    assertSize(yield, 0);

    // turn on
    PowerCommand cmd;
    cmd.pdm(pdm_id);
    cmd.pin(3);
    cmd.cmd(PowerCmd::ON);
    keybox.handle(cmd, yield);

    J1939Message expect_msg(0xEF00, ecm_address, pdm_address, 6);
    expect_msg.data({0x04, 0x1B, 0x01, 0x04, 0x01, 0xFF, 0xFF, 0xFF});

    PowerState expect_event;
    expect_event.pdm(pdm_id);
    expect_event.pin(3);
    expect_event.mode(PowerMode::ON);

    assertSize(yield, 2);
    assertIsJ1939Message(yield.messages()[0], expect_msg);
    assertIsEvent(yield.messages()[1], expect_event);
    yield.clear();

    // turn on again
    keybox.handle(cmd, yield);
    assertSize(yield, 0);
}

test(KeyboxTest, PWM) {
    FakeYield yield;
    uint8_t ecm_address = 0x0A;
    uint8_t pdm_address = 0x21;
    uint8_t pdm_id = 0x01;
    BlinkKeybox keybox(pdm_address, pdm_id);

    // assign source address
    J1939Claim claim(ecm_address, 0);
    keybox.handle(claim, yield);
    assertSize(yield, 0);

    // turn on
    PowerCommand cmd;
    cmd.pdm(pdm_id);
    cmd.pin(11);
    cmd.cmd(PowerCmd::ON);
    keybox.handle(cmd, yield);

    J1939Message expect_msg(0xEF00, ecm_address, pdm_address, 6);
    expect_msg.data({0x04, 0x1B, 0x03, 0xFF, 0x00, 0xFF, 0xFF, 0xFF});

    PowerState expect_event;
    expect_event.pdm(pdm_id);
    expect_event.pin(11);
    expect_event.mode(PowerMode::PWM);
    expect_event.duty_cycle(0xFF);

    assertSize(yield, 2);
    assertIsJ1939Message(yield.messages()[0], expect_msg);
    assertIsEvent(yield.messages()[1], expect_event);
    yield.clear();

    // turn off
    cmd.pdm(pdm_id);
    cmd.pin(11);
    cmd.cmd(PowerCmd::OFF);
    keybox.handle(cmd, yield);

    expect_msg.data({0x04, 0x1B, 0x03, 0x00, 0x00, 0xFF, 0xFF, 0xFF});

    expect_event.pdm(pdm_id);
    expect_event.pin(11);
    expect_event.mode(PowerMode::PWM);
    expect_event.duty_cycle(0x00);

    assertSize(yield, 2);
    assertIsJ1939Message(yield.messages()[0], expect_msg);
    assertIsEvent(yield.messages()[1], expect_event);
    yield.clear();

    // set specific value
    cmd.pdm(pdm_id);
    cmd.pin(11);
    cmd.cmd(PowerCmd::PWM);
    cmd.duty_cycle(0xB3);
    keybox.handle(cmd, yield);

    expect_msg.data({0x04, 0x1B, 0x03, 0xB3, 0x00, 0xFF, 0xFF, 0xFF});

    expect_event.pdm(pdm_id);
    expect_event.pin(11);
    expect_event.mode(PowerMode::PWM);
    expect_event.duty_cycle(0xB3);

    assertSize(yield, 2);
    assertIsJ1939Message(yield.messages()[0], expect_msg);
    assertIsEvent(yield.messages()[1], expect_event);
    yield.clear();

    // set second PWM
    cmd.pdm(pdm_id);
    cmd.pin(12);
    cmd.cmd(PowerCmd::PWM);
    cmd.duty_cycle(0x51);
    keybox.handle(cmd, yield);

    expect_msg.data({0x04, 0x1B, 0x03, 0xB3, 0x51, 0xFF, 0xFF, 0xFF});

    expect_event.pdm(pdm_id);
    expect_event.pin(12);
    expect_event.mode(PowerMode::PWM);
    expect_event.duty_cycle(0x51);

    assertSize(yield, 2);
    assertIsJ1939Message(yield.messages()[0], expect_msg);
    assertIsEvent(yield.messages()[1], expect_event);
    yield.clear();

    // set same value
    cmd.pdm(pdm_id);
    cmd.pin(11);
    cmd.cmd(PowerCmd::PWM);
    cmd.duty_cycle(0xB3);
    keybox.handle(cmd, yield);

    assertSize(yield, 0);
}

test(KeyboxTest, ToggleOutput) {
    FakeYield yield;
    uint8_t ecm_address = 0x0A;
    uint8_t pdm_address = 0x21;
    uint8_t pdm_id = 0x01;
    BlinkKeybox keybox(pdm_address, pdm_id);

    // assign source address
    J1939Claim claim(ecm_address, 0);
    keybox.handle(claim, yield);
    assertSize(yield, 0);

    // toggle on
    PowerCommand cmd;
    cmd.pdm(pdm_id);
    cmd.pin(9);
    cmd.cmd(PowerCmd::TOGGLE);
    keybox.handle(cmd, yield);

    J1939Message expect_msg(0xEF00, ecm_address, pdm_address, 6);
    expect_msg.data({0x04, 0x1B, 0x01, 0x0A, 0x01, 0xFF, 0xFF, 0xFF});

    PowerState expect_event;
    expect_event.pdm(pdm_id);
    expect_event.pin(9);
    expect_event.mode(PowerMode::ON);

    assertSize(yield, 2);
    assertIsJ1939Message(yield.messages()[0], expect_msg);
    assertIsEvent(yield.messages()[1], expect_event);
    yield.clear();

    // toggle off
    cmd.pdm(pdm_id);
    cmd.pin(9);
    cmd.cmd(PowerCmd::TOGGLE);
    keybox.handle(cmd, yield);

    expect_msg.data({0x04, 0x1B, 0x01, 0x0A, 0x00, 0xFF, 0xFF, 0xFF});

    expect_event.pdm(pdm_id);
    expect_event.pin(9);
    expect_event.mode(PowerMode::OFF);

    assertSize(yield, 2);
    assertIsJ1939Message(yield.messages()[0], expect_msg);
    assertIsEvent(yield.messages()[1], expect_event);
    yield.clear();
}

test(KeyboxTest, TogglePWM) {
    FakeYield yield;
    uint8_t ecm_address = 0x0A;
    uint8_t pdm_address = 0x21;
    uint8_t pdm_id = 0x01;
    BlinkKeybox keybox(pdm_address, pdm_id);

    // assign source address
    J1939Claim claim(ecm_address, 0);
    keybox.handle(claim, yield);
    assertSize(yield, 0);

    // turn on
    PowerCommand cmd;
    cmd.pdm(pdm_id);
    cmd.pin(11);
    cmd.cmd(PowerCmd::PWM);
    cmd.duty_cycle(0x15);
    keybox.handle(cmd, yield);

    J1939Message expect_msg(0xEF00, ecm_address, pdm_address, 6);
    expect_msg.data({0x04, 0x1B, 0x03, 0x15, 0x00, 0xFF, 0xFF, 0xFF});

    PowerState expect_event;
    expect_event.pdm(pdm_id);
    expect_event.pin(11);
    expect_event.mode(PowerMode::PWM);
    expect_event.duty_cycle(0x15);

    assertSize(yield, 2);
    assertIsJ1939Message(yield.messages()[0], expect_msg);
    assertIsEvent(yield.messages()[1], expect_event);
    yield.clear();

    // toggle off
    cmd.pdm(pdm_id);
    cmd.pin(11);
    cmd.cmd(PowerCmd::TOGGLE);
    keybox.handle(cmd, yield);

    expect_msg.data({0x04, 0x1B, 0x03, 0x00, 0x00, 0xFF, 0xFF, 0xFF});

    expect_event.pdm(pdm_id);
    expect_event.pin(11);
    expect_event.mode(PowerMode::PWM);
    expect_event.duty_cycle(0x00);

    assertSize(yield, 2);
    assertIsJ1939Message(yield.messages()[0], expect_msg);
    assertIsEvent(yield.messages()[1], expect_event);
    yield.clear();

    // toggle on
    cmd.pdm(pdm_id);
    cmd.pin(11);
    cmd.cmd(PowerCmd::TOGGLE);
    keybox.handle(cmd, yield);

    expect_msg.data({0x04, 0x1B, 0x03, 0xFF, 0x00, 0xFF, 0xFF, 0xFF});

    expect_event.pdm(pdm_id);
    expect_event.pin(11);
    expect_event.mode(PowerMode::PWM);
    expect_event.duty_cycle(0xFF);

    assertSize(yield, 2);
    assertIsJ1939Message(yield.messages()[0], expect_msg);
    assertIsEvent(yield.messages()[1], expect_event);
    yield.clear();

}

test(KeyboxTest, FaultOutput) {
}

test(KeyboxTest, FaultPWM) {
}

}

// Test boilerplate.
void setup() {
#ifdef ARDUINO
    delay(1000);
#endif
    SERIAL_PORT_MONITOR.begin(115200);
    while(!SERIAL_PORT_MONITOR);
}

void loop() {
    TestRunner::run();
    delay(1);
}
