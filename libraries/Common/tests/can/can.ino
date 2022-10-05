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

test(CANNodeTest, NoReads) {
    FakeYield yield;
    FakeConnection can(0, 0);
    CANNode node(&can);

    node.emit(yield);
    assertSize(yield, 0);
}

test(CANNodeTest, Read) {
    FakeYield yield;
    FakeConnection can(1, 0);
    CANNode node(&can);

    Frame f(0x01, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    can.setReadBuffer({f});

    node.emit(yield);
    assertSize(yield, 1);
    assertIsCANFrame(yield.messages()[0], f);
    assertEqual(can.readsRemaining(), 0);
}

test(CANNodeTest, Write) {
    FakeYield yield;
    FakeConnection can(0, 1);
    CANNode node(&can);

    Frame f(0x01, 0, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});
    node.handle(f, yield);
    assertEqual(can.writeCount(), 1);
    assertPrintablesEqual(can.writeData()[0], f);
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
