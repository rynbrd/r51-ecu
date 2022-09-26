#ifndef _R51_COMMON_ENDIAN_H_
#define _R51_COMMON_ENDIAN_H_

#include <Arduino.h>

namespace R51 {

// Return true if the host is little endian.
bool little_endian();

// Read network order bytes into a host order uint32_t.
void NetworkToUInt32(uint32_t* dest, const uint8_t* src);

// Write uint32_t in host byte order to a network order byte array.
void UInt32ToNetwork(uint8_t* dest, const uint32_t src);

}  // namespace R51

#endif // _R51_COMMON_ENDIAN_H_
