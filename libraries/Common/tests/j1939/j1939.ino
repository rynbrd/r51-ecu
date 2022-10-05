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

test(J1939GatewayTest, Read) {
    FakeYield yield;
    FakeConnection can(2, 0);
    J1939Gateway node(&can);

    J1939Message m1(0xEF00, 0x20, 0x10);
    J1939Message m2(0xEF00, 0x20, 0x20);
    can.setReadBuffer({m1, m2});

    node.emit(yield);
    node.emit(yield);
    assertSize(yield, 2);
    assertIsJ1939Message(yield.messages()[0], m1);
    assertIsJ1939Message(yield.messages()[1], m2);
}

test(J1939GatewayTest, Write) {
    FakeYield yield;
    FakeConnection can(0, 1);
    J1939Gateway node(&can);

    J1939Message m(0xEF00, 0x20, 0x10);
    node.handle(m, yield);
    assertEqual(can.writeCount(), 1);
    assertPrintablesEqual(can.writeData()[0], m);
}


test(J1939AddressClaimTest, Init) {
    // Should emit an address claim with the preferred address on first emit.
    FakeYield yield;
    uint8_t address = 0x0A;
    uint64_t name = 0x000013B000FFFAC0;
    J1939AddressClaim node(address, name);


    Frame expect_frame(0x18EEFF0A, 1, {0x00, 0x00, 0x13, 0xB0, 0x00, 0xFF, 0xFA, 0xC0});
    J1939Claim expect_claim(address, name);

    node.init(yield);
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
