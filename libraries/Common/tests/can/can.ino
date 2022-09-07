#include <AUnit.h>
#include <Arduino.h>
#include <Canny.h>
#include <Common.h>
#include <Test.h>

namespace R51 {

using namespace aunit;
using ::Canny::Connection;
using ::Canny::ERR_FIFO;
using ::Canny::ERR_INTERNAL;
using ::Canny::ERR_OK;
using ::Canny::Error;
using ::Canny::Frame;
using ::Canny::J1939Message;

class FakeConnection : public Connection {
    public:
        FakeConnection(size_t read_size, size_t write_size) :
                read_buffer_(nullptr), read_size_(read_size), read_len_(0), read_pos_(0),
                write_buffer_(nullptr), write_size_(write_size), write_len_(0) {
            if (read_size_ > 0) {
                read_buffer_ = new Frame[read_size_];
            }
            if (write_size_ > 0) {
                write_buffer_ = new Frame[write_size_];
            }
        }

        ~FakeConnection() {
            if (read_buffer_ != nullptr) {
                delete[] read_buffer_;
            }
            if (write_buffer_ != nullptr) {
                delete[] write_buffer_;
            }
        }

        Error read(Frame* frame) override {
            if (read_pos_ >= read_len_) {
                return ERR_FIFO;
            }
            *frame = read_buffer_[read_pos_];
            ++read_pos_;
            return ERR_OK; 
        }

        Error write(const Frame& frame) override {
            if (write_len_ >= write_size_) {
                return ERR_FIFO;
            }
            write_buffer_[write_len_++] = frame;
            return ERR_OK;
        }

        Error read(uint32_t*, uint8_t*, uint8_t*, uint8_t*) override { return ERR_INTERNAL; }
        Error write(uint32_t, uint8_t, uint8_t*, uint8_t) override { return ERR_INTERNAL; }

        template <size_t N>
        void setReadBuffer(const Frame (&frames)[N]) {
            for (size_t i = 0; i < N && i < read_size_; ++i) {
                read_buffer_[i] = frames[i];
            }
            read_len_ = N > read_size_ ? read_size_ : N;
            read_pos_ = 0;
        }

        int readsRemaining() {
            return read_len_ - read_pos_;
        }

        Frame* writeData() { return write_buffer_; }
        int writeCount() { return write_len_; }
        void writeReset(int size = -1) {
            if (size >= 0) {
                write_size_ = size;
                delete[] write_buffer_;
                if (size > 0) {
                    write_buffer_ = new Frame[size];
                } else {
                    write_buffer_ = nullptr;
                }
            }
            write_len_ = 0;
        }

    private:
        Frame* read_buffer_;
        size_t read_size_;
        size_t read_len_;
        size_t read_pos_;
        Frame* write_buffer_;
        size_t write_size_;
        size_t write_len_;
};

class TestCANNode : public CANNode<Frame> {
    public:
        TestCANNode(Canny::Connection* can, size_t read_size, size_t write_size) :
                CANNode(can, read_size, write_size) {}

        bool readFilter(const Frame& frame) const override {
            return frame.id() != 0x10;
        }

        bool writeFilter(const Frame& frame) const override {
            return frame.id() != 0x10;
        }
};

class TestJ1939Node : public CANNode<J1939Message> {
    public:
        TestJ1939Node(Canny::Connection* can, size_t read_size, size_t write_size) :
                CANNode(can, read_size, write_size) {}

        bool readFilter(const J1939Message& j1939) const override {
            return j1939.dest_address() == 0x10 || j1939.broadcast();
        }

        bool writeFilter(const J1939Message& j1939) const override {
            return j1939.source_address() == 0x10;
        }
};

test(CANNodeTest, NoReads) {
    FakeYield yield;
    FakeConnection can(0, 0);
    TestCANNode node(&can, 10, 0);

    node.emit(yield);
    assertSize(yield, 0);
}

test(CANNodeTest, ReadOne) {
    FakeYield yield;
    FakeConnection can(1, 0);
    TestCANNode node(&can, 10, 0);

    Frame f(0x01, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    can.setReadBuffer({f});

    node.emit(yield);
    assertSize(yield, 1);
    assertIsCANFrame(yield.messages()[0], f);
    assertEqual(can.readsRemaining(), 0);
}

test(CANNodeTest, ReadMultipleLargerBuffer) {
    FakeYield yield;
    FakeConnection can(3, 0);
    TestCANNode node(&can, 10, 0);

    Frame f1(0x01, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    Frame f2(0x02, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    Frame f3(0x03, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    can.setReadBuffer({f1, f2, f3});

    node.emit(yield);
    assertSize(yield, 1);
    assertIsCANFrame(yield.messages()[0], f1);
    // Node should read everything since there's room in the buffer.
    assertEqual(can.readsRemaining(), 0);
    yield.clear();

    node.emit(yield);
    assertSize(yield, 1);
    assertIsCANFrame(yield.messages()[0], f2);
    yield.clear();

    node.emit(yield);
    assertSize(yield, 1);
    assertIsCANFrame(yield.messages()[0], f3);
    yield.clear();
}

test(CANNodeTest, ReadMultipleSmallerBuffer) {
    FakeYield yield;
    FakeConnection can(3, 0);
    TestCANNode node(&can, 2, 0);

    Frame f1(0x01, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    Frame f2(0x02, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    Frame f3(0x03, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    can.setReadBuffer({f1, f2, f3});

    node.emit(yield);
    assertSize(yield, 1);
    assertIsCANFrame(yield.messages()[0], f1);
    // There should be some since the buffer is too small.
    assertEqual(can.readsRemaining(), 1);
    yield.clear();

    node.emit(yield);
    assertSize(yield, 1);
    assertIsCANFrame(yield.messages()[0], f2);
    assertEqual(can.readsRemaining(), 0);
    yield.clear();

    node.emit(yield);
    assertSize(yield, 1);
    assertIsCANFrame(yield.messages()[0], f3);
    yield.clear();
}

test(CANNodeTest, ReadFiltered) {
    FakeYield yield;
    FakeConnection can(3, 0);
    TestCANNode node(&can, 2, 0);

    Frame f1(0x01, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    Frame f2(0x10, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    Frame f3(0x03, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    can.setReadBuffer({f1, f2, f3});

    node.emit(yield);
    assertSize(yield, 1);
    assertIsCANFrame(yield.messages()[0], f1);
    // There should be none left since there's room in the buffer for unfiltered frames.
    assertEqual(can.readsRemaining(), 0);
    yield.clear();

    node.emit(yield);
    assertSize(yield, 1);
    assertIsCANFrame(yield.messages()[0], f3);
    yield.clear();
}

test(CANNodeTest, WriteNoBuffer) {
    FakeConnection can(0, 1);
    TestCANNode node(&can, 1, 0);

    Frame f(0x01, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    node.handle(f);
    assertEqual(can.writeCount(), 1);
    assertPrintablesEqual(can.writeData()[0], f);
}

test(CANNodeTest, WriteBufferSuccess) {
    FakeConnection can(0, 1);
    TestCANNode node(&can, 1, 1);

    Frame f(0x01, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});

    node.handle(f);
    assertEqual(can.writeCount(), 1);
    assertPrintablesEqual(can.writeData()[0], f);
}

test(CANNodeTest, WriteBufferSomeFIFOAndEmit) {
    FakeYield yield;
    FakeConnection can(0, 1);
    TestCANNode node(&can, 1, 1);

    Frame f1(0x01, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    Frame f2(0x02, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});

    node.handle(f1);
    node.handle(f2);
    assertEqual(can.writeCount(), 1);
    assertPrintablesEqual(can.writeData()[0], f1);
    can.writeReset();

    node.emit(yield);
    assertEqual(can.writeCount(), 1);
    assertPrintablesEqual(can.writeData()[0], f2);
}

test(CANNodeTest, WriteBufferAllErrFIFOAndDrain) {
    FakeYield yield;
    FakeConnection can(0, 0);
    TestCANNode node(&can, 1, 10);

    Frame f1(0x01, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    Frame f2(0x02, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    Frame f3(0x03, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});

    node.handle(f1);
    node.handle(f2);
    assertEqual(can.writeCount(), 0);
    can.writeReset(3);

    node.handle(f3);
    assertEqual(can.writeCount(), 3);
    assertPrintablesEqual(can.writeData()[0], f1);
    assertPrintablesEqual(can.writeData()[1], f2);
    assertPrintablesEqual(can.writeData()[2], f3);
}

test(CANNodeTest, WritteBufferFiltered) {
    FakeYield yield;
    FakeConnection can(0, 0);
    TestCANNode node(&can, 1, 10);

    Frame f1(0x01, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    Frame f2(0x10, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    Frame f3(0x03, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});

    node.handle(f1);
    node.handle(f2);
    assertEqual(can.writeCount(), 0);
    can.writeReset(3);

    node.handle(f3);
    assertEqual(can.writeCount(), 2);
    assertPrintablesEqual(can.writeData()[0], f1);
    assertPrintablesEqual(can.writeData()[1], f3);
}

test(CANNodeTest, J1939) {
    FakeYield yield;
    FakeConnection can(3, 0);
    TestJ1939Node node(&can, 1, 10);

    J1939Message m1(0xEF00, 0x20, 0x10);
    J1939Message m3(0xEF00, 0x20, 0x20);
    J1939Message m2(0xFF00, 0x20);
    can.setReadBuffer({m1, m2, m3});

    node.emit(yield);
    node.emit(yield);
    node.emit(yield);
    assertSize(yield, 2);
    assertIsJ1939Message(yield.messages()[0], m1);
    assertIsJ1939Message(yield.messages()[1], m2);
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
