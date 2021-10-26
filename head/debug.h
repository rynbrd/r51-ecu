#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <Stream.h>
#include <stdint.h>


#ifdef DEBUG_STREAM
extern Stream* Debug = &DEBUG_STREAM;

size_t printDebugFrame(uint32_t id, uint8_t len, uint8_t* data);

#define INFO_MSG(MSG) ({\
    Debug->print("[INFO]  ");\
    Debug->println(MSG);\
})
#define INFO_MSG_VAL(MSG, VAL) ({\
    Debug->print("[INFO]  ");\
    Debug->print(MSG);\
    Debug->println(VAL);\
})
#define INFO_MSG_VAL_FMT(MSG, VAL, FMT) ({\
    Debug->print("[INFO]  ");\
    Debug->print(MSG);\
    Debug->println(VAL, FMT);\
})
#define INFO_MSG_FRAME(MSG, ID, LEN, DATA) ({\
    Debug->print("[INFO]  ");\
    Debug->print(MSG);\
    Debug->printDebugFrame(ID, LEN, Data);\
    Debug->println("");\
})

#define ERROR_MSG(MSG) ({\
    Debug->print("[ERROR] ");\
    Debug->println(MSG);\
})
#define ERROR_MSG_VAL(MSG, VAL) ({\
    Debug->print("[ERROR] ");\
    Debug->print(MSG);\
    Debug->println(VAL);\
})
#define ERROR_MSG_VAL_FMT(MSG, VAL, FMT) ({\
    Debug->print("[ERROR] ");\
    Debug->print(MSG);\
    Debug->println(VAL, FMT);\
})
#define ERROR_MSG_FRAME(MSG, ID, LEN, DATA) ({\
    Debug->print("[ERROR] ");\
    Debug->print(MSG);\
    Debug->printDebugFrame(ID, LEN, Data);\
    Debug->println("");\
})

#else

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
