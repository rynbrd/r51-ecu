#ifndef __R51_BRIDGE_DEBUG_H__
#define __R51_BRIDGE_DEBUG_H__

#include <Arduino.h>

#include "Config.h"

#if defined(DEBUG_ENABLE) && defined(SERIAL_DEVICE)

#define D(x) x

#define DEBUG_MSG(MSG) ({\
    SERIAL_DEVICE.println(MSG);\
    SERIAL_DEVICE.flush();\
})
#define DEBUG_MSG_VAL(MSG, VAL) ({\
    SERIAL_DEVICE.print(MSG);\
    SERIAL_DEVICE.println(VAL);\
    SERIAL_DEVICE.flush();\
})
#define DEBUG_MSG_VAL_FMT(MSG, VAL, FMT) ({\
    SERIAL_DEVICE.print(MSG);\
    SERIAL_DEVICE.println(VAL, FMT);\
    SERIAL_DEVICE.flush();\
})

#else

#define D(x)

#define DEBUG_MSG(MSG)
#define DEBUG_MSG_VAL(MSG, VAL)
#define DEBUG_MSG_VAL_FMT(MSG, VAL, FMT)

#endif  // DEBUG_ENABLE && SERIAL_DEVICE

#endif  // __R51_BRIDGE_DEBUG_H__
