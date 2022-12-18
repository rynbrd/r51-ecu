#include <AUnit.h>
#include <Arduino.h>
#include <Canny.h>
#include <Core.h>
#include <Test.h>

namespace R51 {

using namespace aunit;
using ::Canny::CAN20Frame;
using ::Canny::Connection;
using ::Canny::ERR_FIFO;
using ::Canny::ERR_INTERNAL;
using ::Canny::ERR_OK;
using ::Canny::Error;
using ::Canny::J1939Message;

class FakeConnection : public Connection<J1939Message> {
    public:
        FakeConnection(size_t read_size, size_t write_size) :
                read_buffer_(nullptr), read_size_(read_size), read_len_(0), read_pos_(0),
                write_buffer_(nullptr), write_size_(write_size), write_len_(0) {
            if (read_size_ > 0) {
                read_buffer_ = new J1939Message[read_size_];
            }
            if (write_size_ > 0) {
                write_buffer_ = new J1939Message[write_size_];
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

        Error read(J1939Message* frame) override {
            if (read_pos_ >= read_len_) {
                return ERR_FIFO;
            }
            *frame = read_buffer_[read_pos_];
            ++read_pos_;
            return ERR_OK; 
        }

        Error write(const J1939Message& frame) override {
            if (write_len_ >= write_size_) {
                return ERR_FIFO;
            }
            write_buffer_[write_len_++] = frame;
            return ERR_OK;
        }

        template <size_t N>
        void setReadBuffer(const J1939Message (&frames)[N]) {
            for (size_t i = 0; i < N && i < read_size_; ++i) {
                read_buffer_[i] = frames[i];
            }
            read_len_ = N > read_size_ ? read_size_ : N;
            read_pos_ = 0;
        }

        int readsRemaining() {
            return read_len_ - read_pos_;
        }

        J1939Message* writeData() { return write_buffer_; }
        int writeCount() { return write_len_; }
        void writeReset(int size = -1) {
            if (size >= 0) {
                write_size_ = size;
                delete[] write_buffer_;
                if (size > 0) {
                    write_buffer_ = new J1939Message[size];
                } else {
                    write_buffer_ = nullptr;
                }
            }
            write_len_ = 0;
        }

    private:
        J1939Message* read_buffer_;
        size_t read_size_;
        size_t read_len_;
        size_t read_pos_;
        J1939Message* write_buffer_;
        size_t write_size_;
        size_t write_len_;
};

test(J1939GatewayTest, ReadFiltered) {
    FakeYield yield;
    FakeConnection can(3, 0);
    J1939Gateway node(&can, 0x10, false);

    J1939Message m1(0xEF00, 0x20, 0x10);
    J1939Message m2(0xEF00, 0x30, 0x20);
    J1939Message m3(0xEF00, 0x40, 0xFF);
    can.setReadBuffer({m1, m2, m3});

    node.emit(yield);
    node.emit(yield);
    node.emit(yield);
    assertSize(yield, 2);
    assertIsJ1939Message(yield.messages()[0], m1);
    assertIsJ1939Message(yield.messages()[1], m3);
}

test(J1939GatewayTest, ReadPromiscuous) {
    FakeYield yield;
    FakeConnection can(3, 0);
    J1939Gateway node(&can, 0x10, true);

    J1939Message m1(0xEF00, 0x20, 0x10);
    J1939Message m2(0xEF00, 0x30, 0x20);
    J1939Message m3(0xEF00, 0x40, 0xFF);
    can.setReadBuffer({m1, m2, m3});

    node.emit(yield);
    node.emit(yield);
    node.emit(yield);
    assertSize(yield, 3);
    assertIsJ1939Message(yield.messages()[0], m1);
    assertIsJ1939Message(yield.messages()[1], m2);
    assertIsJ1939Message(yield.messages()[2], m3);
}

test(J1939GatewayTest, WriteFiltered) {
    FakeYield yield;
    FakeConnection can(0, 3);
    J1939Gateway node(&can, 0x10, false);

    J1939Message m1(0xEF00, 0x10, 0x11);
    J1939Message m2(0xEF00, 0x10, 0xFF);
    J1939Message m3(0xEF00, 0x11, 0x13);
    node.handle(MessageView(&m1), yield);
    node.handle(MessageView(&m2), yield);
    node.handle(MessageView(&m3), yield);
    assertSize(yield, 0);
    assertEqual(can.writeCount(), 2);
    assertPrintablesEqual(can.writeData()[0], m1);
    assertPrintablesEqual(can.writeData()[1], m2);
}

test(J1939GatewayTest, WritePromiscuous) {
    FakeYield yield;
    FakeConnection can(0, 3);
    J1939Gateway node(&can, 0x10, true);

    J1939Message m1(0xEF00, 0x10, 0x11);
    J1939Message m2(0xEF00, 0x10, 0xFF);
    J1939Message m3(0xEF00, 0x11, 0x13);
    node.handle(MessageView(&m1), yield);
    node.handle(MessageView(&m2), yield);
    node.handle(MessageView(&m3), yield);
    assertSize(yield, 0);
    assertEqual(can.writeCount(), 3);
    assertPrintablesEqual(can.writeData()[0], m1);
    assertPrintablesEqual(can.writeData()[1], m2);
    assertPrintablesEqual(can.writeData()[2], m3);
}

test(J1939GatewayTest, InitAnnounce) {
    // Should emit an address claim with the preferred address on first emit.
    FakeYield yield;
    FakeConnection can(0, 3);
    uint8_t address = 0x0A;
    uint64_t name = 0x000013B000FFFAC0;
    J1939Gateway node(&can, address, name, false);

    CAN20Frame expect_frame(0x18EEFF0A, 1, {0x00, 0x00, 0x13, 0xB0, 0x00, 0xFF, 0xFA, 0xC0});
    J1939Claim expect_claim(address, name);

    node.init(yield);
    assertEqual(can.writeCount(), 1);
    assertPrintablesEqual(can.writeData()[0], expect_frame);
    assertSize(yield, 1);
    assertIsJ1939Claim(yield.messages()[0], expect_claim);
}

test(J1939GatewayTest, InitHardCode) {
    // Should emit only an internal message.
    FakeYield yield;
    FakeConnection can(0, 3);
    uint8_t address = 0x0A;
    J1939Gateway node(&can, address, false);

    J1939Claim expect_claim(address, 0);

    node.init(yield);
    assertEqual(can.writeCount(), 0);
    assertSize(yield, 1);
    assertIsJ1939Claim(yield.messages()[0], expect_claim);
}

test(J1939GatewayTest, RequestAddressClaim) {
    FakeYield yield;
    FakeConnection can(1, 1);
    uint8_t address = 0x0A;
    uint64_t name = 0x000013B000FFFAC0;
    J1939Gateway node(&can, address, name, false);

    // send initial claim
    node.init(yield);
    yield.clear();

    // handle request address claim
    J1939Message msg(0xEA00, 0x21, 0xFF, 0x06);
    msg.data({0x00, 0xEE, 0x00});
    can.setReadBuffer({msg});
    node.emit(yield);

    // we should respond with our address
    CAN20Frame expect_frame(0x18EEFF0A, 1, {0x00, 0x00, 0x13, 0xB0, 0x00, 0xFF, 0xFA, 0xC0});

    node.emit(yield);
    assertEqual(can.writeCount(), 1);
    assertPrintablesEqual(can.writeData()[0], expect_frame);
    assertSize(yield, 1);
    assertIsJ1939Message(yield.messages()[0], msg);
}

test(J1939GatewayTest, NoArbitraryAddressCannotClaim) {
    FakeYield yield;
    FakeConnection can(1, 1);
    uint8_t address = 0x0A;
    uint64_t name = 0x000013B000FFFAC0;
    J1939Gateway node(&can, address, name, false);

    // send initial claim
    node.init(yield);
    yield.clear();
    can.writeReset();

    // handle address claim with same address and higher priority name
    J1939Message msg(0xEA00, address, 0xFF, 0x06);
    msg.data({0x00, 0x00, 0x0B, 0xB0, 0x00, 0xFF, 0xFA, 0xC0});
    can.setReadBuffer({msg});
    node.emit(yield);

    // we should respond with a null address
    CAN20Frame expect_frame(0x18EEFFFE, 1, {0x00, 0x00, 0x13, 0xB0, 0x00, 0xFF, 0xFA, 0xC0});
    J1939Claim expect_claim(Canny::NullAddress, name);

    assertEqual(can.writeCount(), 1);
    assertPrintablesEqual(can.writeData()[0], expect_frame);
    assertSize(yield, 2);
    assertIsJ1939Claim(yield.messages()[0], expect_claim);
    assertIsJ1939Message(yield.messages()[1], msg);
}

test(J1939GatewayTest, ArbitraryAddressClaim) {
    FakeYield yield;
    FakeConnection can(1, 1);
    uint8_t address = 0x0A;
    uint64_t name = 0x000013B000FFFAC1;
    J1939Gateway node(&can, address, name, false);

    // send initial claim
    node.emit(yield);
    yield.clear();
    can.writeReset();

    // handle address claim with same address and higher priority name
    J1939Message msg(0xEA00, address, 0xFF, 0x06);
    msg.data({0x00, 0x00, 0x0B, 0xB0, 0x00, 0xFF, 0xFA, 0xC0});
    can.setReadBuffer({msg});
    node.emit(yield);

    // we should respond with the next address
    CAN20Frame expect_frame(0x18EEFF0B, 1, {0x00, 0x00, 0x13, 0xB0, 0x00, 0xFF, 0xFA, 0xC1});
    J1939Claim expect_claim(0x0B, name);

    assertEqual(can.writeCount(), 1);
    assertPrintablesEqual(can.writeData()[0], expect_frame);
    assertSize(yield, 2);
    assertIsJ1939Claim(yield.messages()[0], expect_claim);
    assertIsJ1939Message(yield.messages()[1], msg);
}

test(J1939GatewayTest, ArbitraryAddressCannotClaim) {
    FakeYield yield;
    FakeConnection can(1, 1);
    uint8_t address = 0x0A;
    uint64_t name = 0x000013B000FFFAC1;
    J1939Gateway node(&can, address, name, false);

    // send initial claim
    node.emit(yield);
    yield.clear();
    can.writeReset();

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
        can.setReadBuffer({msg});
        node.emit(yield);

        // we should response with the next address
        CAN20Frame expect_frame(0x18EEFF00 | next, 1, {0x00, 0x00, 0x13, 0xB0, 0x00, 0xFF, 0xFA, 0xC1});
        J1939Claim expect_claim(next, name);

        assertEqual(can.writeCount(), 1);
        assertPrintablesEqual(can.writeData()[0], expect_frame);
        assertSize(yield, 2);
        assertIsJ1939Claim(yield.messages()[0], expect_claim);
        assertIsJ1939Message(yield.messages()[1], msg);

        yield.clear();
        can.writeReset();

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
