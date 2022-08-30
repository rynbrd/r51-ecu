#ifndef _R51_VEHICLE_UNITS_H_
#define _R51_VEHICLE_UNITS_H_

#include <Arduino.h>

namespace R51 {

// Measurement units.
enum Units : uint8_t {
    UNITS_METRIC = 0,
    UNITS_US = 1,
};

}  // namespace R51

#endif  // _R51_VEHICLE_UNITS_H_
