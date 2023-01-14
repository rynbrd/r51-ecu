#ifndef _R51_TOOLS_ROTARY_ENCODER_DEBUG_H_
#define _R51_TOOLS_ROTARY_ENCODER_DEBUG_H_

#include "Config.h"
#include <Arduino.h>

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
#define DEBUG_MSG_OBJ(MSG, OBJ) ({\
    SERIAL_DEVICE.print(MSG);\
    (OBJ).printTo(SERIAL_DEVICE);\
    SERIAL_DEVICE.println();\
    SERIAL_DEVICE.flush();\
})
#define DEBUG_MSG_FMT(MSG, VAL, FMT) ({\
    SERIAL_DEVICE.print(MSG);\
    SERIAL_DEVICE.println(VAL, FMT);\
    SERIAL_DEVICE.flush();\
})

#endif  // _R51_TOOLS_ROTARY_ENCODER_DEBUG_H_
