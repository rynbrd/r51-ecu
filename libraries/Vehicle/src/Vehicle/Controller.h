#ifndef _R51_VEHICLE_CONTROLLER_H_
#define _R51_VEHICLE_CONTROLLER_H_

#include <Canny.h>

namespace R51 {

// Controller manages a frame that is sent to control connected CAN devices.
class Controller {
    public:
        Controller() = default;
        virtual ~Controller() = default;

        // Return true if the control frame has available changes that should
        // be sent immediately. This is reset to false after a call to frame().
        virtual bool available() = 0;

        // Return a reference to the control frame. The lifecycle of the frame
        // is tied to the controller that returns it.
        virtual const Canny::Frame& frame() = 0;
};

}  // namespace R51

#endif  // _R51_VEHICLE_CONTROLLER_H_
