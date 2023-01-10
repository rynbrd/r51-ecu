#ifndef _R51_PICO_H_
#define _R51_PICO_H_

#if !defined(PICO_RP2040) && !defined(ARDUINO_RASPBERRY_PI_PICO)
#error "Platform not supported, must be RP2040."
#endif

#include <Platform/Config.h>
#include <Platform/Pipe.h>

#endif  // _R51_PICO_H_
