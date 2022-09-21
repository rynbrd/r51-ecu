#include "Scratch.h"

#include <Arduino.h>

namespace R51::internal {

void ScratchSetCommand::run(Console* console, char* arg, const Caster::Yield<Message>&) {
    size_t len = strlen(arg);
    if (len + 1 > kScratchCapacity) {
        console->stream()->println("console: scratch buffer overflow");
        return;
    }
    strcpy((char*)scratch_->bytes, arg);
    scratch_->size = len + 1;
}

}  // namespace R51::internal
