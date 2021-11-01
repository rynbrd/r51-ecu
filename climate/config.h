#ifndef __R51_CONFIG__
#define __R51_CONFIG__

// Uncomment the following line to enable debug output.
//#define DEBUG_ENABLE
#define DEBUG_SERIAL Serial
#define DEBUG_BAUDRATE 115200

// RealDash serial interface. This should not require a DTR to begin as
// RealDash does not send a DTR. On CanBed (and other Leonardos) this needs to
// be the Serial1 TTL as the USB connected Serial expects a DTR before
// transmitting. 
#define REALDASH_SERIAL Serial1
#define REALDASH_BAUDRATE 115200

// CAN Bus configuration. Set the appropriate CS pin for the target board.
// Baudrate should match the connected CAN bus.
#define CAN_CS_PIN 17
#define CAN_BAUDRATE CAN_500KBPS

#endif  // __R51_CONFIG__
