#ifndef __R51_CONTROLLER_H__
#define __R51_CONTROLLER_H__

// Base class for hardware controllers.
class Controller {
    public:
        Controller() {}
        virtual ~Controller() {}

        // Push state changes to the hardware.
        virtual void push() = 0;
};

#endif  // __R51_CONTROLLER_H__

