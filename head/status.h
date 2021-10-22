#ifndef __ECU_STATUS_H__
#define __ECU_STATUS_H__

#include <stdint.h>
#include <Stream.h>

namespace ECU {

// Returned to indicate the resulting status of an operation.
typedef enum : int8_t {
    OK,
    NOENT,
    ERROR,
} Status;

size_t printStatus(Stream* stream, Status status);

}

#endif
