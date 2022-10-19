#ifndef _R51_COMMON_SCRATCH_H_
#define _R51_COMMON_SCRATCH_H_

namespace R51 {

static const size_t kScratchCapacity = 256;

// A scratch space for temporary storage that's easily pre-allocated.
struct Scratch {
    // The byte buffer that holds the data in the space.
    uint8_t bytes[kScratchCapacity];
    // The number of bytes currently occupying the spacce.
    size_t size;

    // Create a new buffer.
    Scratch() : size(0) {
        memset(bytes, 0, kScratchCapacity);
    }

    // Clear the buffer.
    void clear() {
        size = 0;
        bytes[0] = 0;
    }
};

}  // namespace R51

#endif  // _R51_COMMON_SCRATCH_H_
