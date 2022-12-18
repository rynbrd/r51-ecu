#include <AUnit.h>
#include <Arduino.h>
#include <Caster.h>
#include <Core.h>
#include <Test.h>

namespace R51 {

using namespace aunit;
using ::Canny::CAN20Frame;
using ::Canny::J1939Message;
using ::Caster::Bus;
using ::Caster::Node;
using ::Caster::Yield;

class PicoPipeTestImpl : public PicoPipe {
    public:
        PicoPipeTestImpl(size_t left_capacity, size_t right_capacity) :
            PicoPipe(left_capacity, right_capacity) {}

        bool filterLeft(const Message& msg) override {
            return msg.type() != Message::EVENT || msg.event()->id < 0x10;
        }

        bool filterRight(const Message& msg) override {
            return msg.type() != Message::EVENT || msg.event()->id > 0x10;
        }

        void onBufferOverrun(const Message& msg) override {
            ++overruns;
        }

        uint8_t overruns = 0;
};

template <size_t N>
class FakeNode : public Node<Message> {
    public:
        FakeNode() : size(0), capacity(N) {}

        void handle(const Message& msg, const Yield<Message>&) override {
            if (size >= capacity) {
                return;
            }
            messages[size++] = msg;
        }

        MessageValue messages[N];
        size_t size;
        size_t capacity;
};

test(PicoPipeTest, TransferLeftToRight) {
    Event event(0x01, 0x02);
    MessageView msg(&event);
    FakeNode<2> fake;

    PicoPipe smp(1, 1);
    Node<Message>* left_nodes[] = {smp.left()};
    Caster::Bus<Message> left_bus(left_nodes, sizeof(left_nodes)/sizeof(left_nodes[0]));
    Node<Message>* right_nodes[] = {smp.right(), &fake};
    Caster::Bus<Message> right_bus(right_nodes, sizeof(right_nodes)/sizeof(right_nodes[0]));

    left_bus.emit(msg);
    right_bus.loop();

    assertEqual((int)fake.size, 1);
    assertPrintablesEqual(fake.messages[0], msg);
}

test(PicoPipeTest, TransferRightToLeft) {
    Event event(0x13, 0x04);
    MessageView msg(&event);
    FakeNode<2> fake;

    PicoPipe smp(1, 1);
    Node<Message>* left_nodes[] = {smp.left(), &fake};
    Caster::Bus<Message> left_bus(left_nodes, sizeof(left_nodes)/sizeof(left_nodes[0]));
    Node<Message>* right_nodes[] = {smp.right()};
    Caster::Bus<Message> right_bus(right_nodes, sizeof(right_nodes)/sizeof(right_nodes[0]));

    right_bus.emit(msg);
    left_bus.loop();

    assertEqual((int)fake.size, 1);
    assertPrintablesEqual(fake.messages[0], msg);
}

test(PicoPipeTest, FilterLeft) {
    Event event1(0x01, 0x02);
    MessageView msg1(&event1);
    Event event2(0x01, 0x12);
    MessageView msg2(&event2);
    FakeNode<2> fake;

    PicoPipeTestImpl smp(2, 2);
    Node<Message>* left_nodes[] = {smp.left()};
    Caster::Bus<Message> left_bus(left_nodes, sizeof(left_nodes)/sizeof(left_nodes[0]));
    Node<Message>* right_nodes[] = {smp.right(), &fake};
    Caster::Bus<Message> right_bus(right_nodes, sizeof(right_nodes)/sizeof(right_nodes[0]));

    left_bus.emit(msg1);
    right_bus.loop();
    left_bus.emit(msg2);
    right_bus.loop();

    assertEqual((int)fake.size, 1);
    assertPrintablesEqual(fake.messages[0], msg1);
}

test(PicoPipeTest, FilterRight) {
    Event event1(0x01, 0x02);
    MessageView msg1(&event1);
    Event event2(0x01, 0x12);
    MessageView msg2(&event2);
    FakeNode<2> fake;

    PicoPipeTestImpl smp(2, 2);
    Node<Message>* left_nodes[] = {smp.left(), &fake};
    Caster::Bus<Message> left_bus(left_nodes, sizeof(left_nodes)/sizeof(left_nodes[0]));
    Node<Message>* right_nodes[] = {smp.right()};
    Caster::Bus<Message> right_bus(right_nodes, sizeof(right_nodes)/sizeof(right_nodes[0]));

    right_bus.emit(msg1);
    left_bus.loop();
    right_bus.emit(msg2);
    left_bus.loop();

    assertEqual((int)fake.size, 1);
    assertPrintablesEqual(fake.messages[0], msg2);
}

test(PicoPipeTest, Overrun) {
    Event event1(0x01, 0x01);
    MessageView msg1(&event1);
    Event event2(0x01, 0x02);
    MessageView msg2(&event2);
    Event event3(0x01, 0x03);
    MessageView msg3(&event3);
    FakeNode<2> fake;

    Serial.println(">>> start");
    PicoPipeTestImpl smp(1, 1);
    Node<Message>* left_nodes[] = {smp.left()};
    Caster::Bus<Message> left_bus(left_nodes, sizeof(left_nodes)/sizeof(left_nodes[0]));
    Node<Message>* right_nodes[] = {smp.right(), &fake};
    Caster::Bus<Message> right_bus(right_nodes, sizeof(right_nodes)/sizeof(right_nodes[0]));

    left_bus.emit(msg1);
    left_bus.emit(msg2);
    left_bus.emit(msg3);
    right_bus.loop();
    right_bus.loop();
    right_bus.loop();
    Serial.println("<<< stop");

    assertEqual((int)fake.size, 1);
    assertPrintablesEqual(fake.messages[0], msg1);
    assertEqual(smp.overruns, 2);
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
