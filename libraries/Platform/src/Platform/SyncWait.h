#ifndef _R51_PLATFORM_SYNC_WAIT_H_
#define _R51_PLATFORM_SYNC_WAIT_H_

#include <Arduino.h>

extern "C" {
    #include <hardware/sync.h>
};

namespace R51 {

// A class which waits for all cores on a RP2040 to reach a point in execution
// before allowing them all to continue executing. This is used primarily to
// allow setup() and setup1() to complete before allowing the two loop
// functions to begin.
class SyncWait  {
    public:
        // Construct a new SyncWait.
        SyncWait();

        // Block until the other core calls wait(). The second core to call
        // wait() will not block.
        void wait();
    private:
        uint8_t tickets_;
        spin_lock_t* lock_;
};

}  // namespace R51

#endif  // _R51_PLATFORM_SYNC_WAIT_H_
