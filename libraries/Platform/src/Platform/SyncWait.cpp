#include "SyncWait.h"

#include <Arduino.h>

extern "C" {
    #include <hardware/sync.h>
};

namespace R51 {

SyncWait::SyncWait() : tickets_(2) {
    lock_ = spin_lock_instance(next_striped_spin_lock_num());
}

void SyncWait::wait() {
    bool spin = false;
    while (true) {
        uint32_t save = spin_lock_blocking(lock_);
        if (tickets_ <= 0) {
            // no cores waiting, unlock and return
            spin_unlock(lock_, save);
            break;
        } else if (!spin) {
            // decrement tickets and continue
            --tickets_;
            spin = true;
        }
        spin_unlock(lock_, save);
    }
}

}  // namespace R51
