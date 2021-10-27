#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <Stream.h>
#include <stdint.h>


#ifdef DEBUG_SERIAL

size_t printDebugFrame(uint32_t id, uint8_t len, uint8_t* data) {
    size_t n = 0;
    n += DEBUG_SERIAL.print(id, HEX);
    n += DEBUG_SERIAL.print("#");
    for (int i = 0; i < len; i++) {
        if (data[i] <= 0x0F) {
            n += DEBUG_SERIAL.print("0");
        }
        n += DEBUG_SERIAL.print(data[i], HEX);
        if (i < len-1) {
            n += DEBUG_SERIAL.print(":");
        }
    }
    return n;
}

#define DEBUG_BEGIN(BAUD) DEBUG_SERIAL.begin(BAUD)

#define INFO_MSG(MSG) ({\
    DEBUG_SERIAL.print("[INFO]  ");\
    DEBUG_SERIAL.println(MSG);\
})
#define INFO_MSG_VAL(MSG, VAL) ({\
    DEBUG_SERIAL.print("[INFO]  ");\
    DEBUG_SERIAL.print(MSG);\
    DEBUG_SERIAL.println(VAL);\
})
#define INFO_MSG_VAL_FMT(MSG, VAL, FMT) ({\
    DEBUG_SERIAL.print("[INFO]  ");\
    DEBUG_SERIAL.print(MSG);\
    DEBUG_SERIAL.println(VAL, FMT);\
})
#define INFO_MSG_FRAME(MSG, ID, LEN, DATA) ({\
    DEBUG_SERIAL.print("[INFO]  ");\
    DEBUG_SERIAL.print(MSG);\
    printDebugFrame(ID, LEN, DATA);\
    DEBUG_SERIAL.println("");\
})

#define ERROR_MSG(MSG) ({\
    DEBUG_SERIAL.print("[ERROR] ");\
    DEBUG_SERIAL.println(MSG);\
})
#define ERROR_MSG_VAL(MSG, VAL) ({\
    DEBUG_SERIAL.print("[ERROR] ");\
    DEBUG_SERIAL.print(MSG);\
    DEBUG_SERIAL.println(VAL);\
})
#define ERROR_MSG_VAL_FMT(MSG, VAL, FMT) ({\
    DEBUG_SERIAL.print("[ERROR] ");\
    DEBUG_SERIAL.print(MSG);\
    DEBUG_SERIAL.println(VAL, FMT);\
})
#define ERROR_MSG_FRAME(MSG, ID, LEN, DATA) ({\
    DEBUG_SERIAL.print("[ERROR] ");\
    DEBUG_SERIAL.print(MSG);\
    printDebugFrame(ID, LEN, DATA);\
    DEBUG_SERIAL.println("");\
})

#else

#define DEBUG_BEGIN(BAUD)
#define DEBUG_BEGIN(BAUD, CONFIG)

#define INFO_MSG(MSG)
#define INFO_MSG_VAL(MSG, VAL)
#define INFO_MSG_VAL_FMT(MSG, VAL, FMT)
#define INFO_MSG_FRAME(MSG, ID, LEN, DATA)

#define ERROR_MSG(MSG)
#define ERROR_MSG_VAL(MSG, VAL)
#define ERROR_MSG_VAL_FMT(MSG, VAL, FMT)
#define ERROR_FRAME(MSG, ID, LEN, DATA)

#endif

#endif
