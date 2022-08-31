#ifndef __R51_BRIDGE_DEBUG_H__
#define __R51_BRIDGE_DEBUG_H__

#include <Arduino.h>

#include "Config.h"

static const char kInfoPrefix[] = "[I] ";
static const char kErrorPrefix[] = "[E] ";

#if defined(DEBUG_ENABLE) && defined(SERIAL_DEVICE)

#define D(x) x

#define INFO_MSG(MSG) ({\
    SERIAL_DEVICE.print(kInfoPrefix);\
    SERIAL_DEVICE.println(MSG);\
    SERIAL_DEVICE.flush();\
})
#define INFO_MSG_VAL(MSG, VAL) ({\
    SERIAL_DEVICE.print(kInfoPrefix);\
    SERIAL_DEVICE.print(MSG);\
    SERIAL_DEVICE.println(VAL);\
    SERIAL_DEVICE.flush();\
})
#define INFO_MSG_VAL_FMT(MSG, VAL, FMT) ({\
    SERIAL_DEVICE.print(kInfoPrefix);\
    SERIAL_DEVICE.print(MSG);\
    SERIAL_DEVICE.println(VAL, FMT);\
    SERIAL_DEVICE.flush();\
})

#define ERROR_MSG(MSG) ({\
    SERIAL_DEVICE.print(kErrorPrefix);\
    SERIAL_DEVICE.println(MSG);\
    SERIAL_DEVICE.flush();\
})
#define ERROR_MSG_VAL(MSG, VAL) ({\
    SERIAL_DEVICE.print(kErrorPrefix);\
    SERIAL_DEVICE.print(MSG);\
    SERIAL_DEVICE.println(VAL);\
    SERIAL_DEVICE.flush();\
})
#define ERROR_MSG_VAL_FMT(MSG, VAL, FMT) ({\
    SERIAL_DEVICE.print(kErrorPrefix);\
    SERIAL_DEVICE.print(MSG);\
    SERIAL_DEVICE.println(VAL, FMT);\
    SERIAL_DEVICE.flush();\
})

#else

#define D(x)

#define INFO_MSG(MSG)
#define INFO_MSG_VAL(MSG, VAL)
#define INFO_MSG_VAL_FMT(MSG, VAL, FMT)

#define ERROR_MSG(MSG)
#define ERROR_MSG_VAL(MSG, VAL)
#define ERROR_MSG_VAL_FMT(MSG, VAL, FMT)

#endif  // DEBUG_ENABLE && SERIAL_DEVICE

#endif  // __R51_BRIDGE_DEBUG_H__
