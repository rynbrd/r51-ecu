#ifndef _R51_CONSOLE_READER_
#define _R51_CONSOLE_READER_

#include <Arduino.h>

namespace R51 {

// Reader reads formatted data from a stream.
class Reader {
    public:
        enum Error {
            EOW,
            EOL,
            FIFO,
            OVERRUN,
        };

        // Construct a reader which reads bytes from the provided stream and
        // writes results to a buffer with given size.
        Reader(Stream* stream, char* buffer, size_t size) :
            stream_(stream), buffer_(buffer), size_(size), len_(0), space_(0) {}

        // Read a word into a buffer without blocking. Return EOW when a word
        // is read into the buffer and there are more words to read on the
        // current line. Return EOL if a word is read into the buffer and that
        // is the last word on the line. Reutrn FIFO if no data is available to
        // be read from the stream. Return OVERRUN if a buffer overrun occurs.
        // Leading and trailing whitespace is discarded.
        Error word(size_t* len = nullptr);

        // Read the remainder of a line into a buffer without blocking. Return
        // EOL when a line is available in the buffer. Return FIFO if no data is
        // available to be read from the stream. Return OVERRUN of a buffer
        // overrun occurs. Leading and trailing whitespace is discarded.
        Error line(size_t* len = nullptr);

        // Reset internal reader state. Useful after an external error occurs.
        // Optionally set a new buffer to write to.
        void reset(char* buffer = nullptr, size_t size = 0);

    private:
        Error read(size_t* len, bool line);

        Stream* stream_;
        char* buffer_;
        size_t size_;
        size_t len_;
        size_t space_;
};

}  // namespace R51

#endif  // _R51_CONSOLE_READER_
