#ifndef _R51_VEHICLE_HANDLER_H_
#define _R51_VEHICLE_HANDLER_H_

#include <Canny.h>

namespace R51 {

// Handle CAN state update frames.
class Handler {
    public:
        Handler() = default;
        virtual ~Handler() = default;

        // Handle a state update CAN frame. Return true if the frame modified
        // the current state.
        virtual bool handle(const Canny::Frame& frame) = 0;
};

}  // namespace R51

#endif  // _R51_VEHICLE_HANDLER_H_
