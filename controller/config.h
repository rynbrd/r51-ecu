#ifndef __R51_CONFIG__
#define __R51_CONFIG__

// Rear defrost is controlled via a mosfet connected to a digital pin. The
// mosfet is enabled momentarily to act as if it were a momentary button press
// by a human. The pin that controls the mosfet and how long the "button" is
// pressed is configured here.
#define REAR_DEFROST_PIN 6
#define REAR_DEFROST_TRIGGER_MS 200

// Uncomment the following line to enable debug output.
//#define DEBUG_ENABLE
#define DEBUG_SERIAL Serial
#define DEBUG_BAUDRATE 115200
// Uncomment to block boot until serial is connected.
//#define DEBUG_WAIT_FOR_SERIAL

// RealDash serial interface. This should not require a DTR to begin as
// RealDash does not send a DTR. On CanBed (and other Leonardos) this needs to
// be the Serial1 TTL as the USB connected Serial expects a DTR before
// transmitting. 
#define REALDASH_SERIAL Serial1
#define REALDASH_BAUDRATE 57600
#define REALDASH_REPEAT 3

// CAN Bus configuration. Set the appropriate CS pin for the target board.
// Baudrate should match the connected CAN bus.
#define CAN_BAUDRATE CAN_500KBPS
#define CAN_CLOCK MCP_16MHZ
// Uncomment to disable writes to the CAN bus.
//#define CAN_LISTEN_ONLY

#endif  // __R51_CONFIG__
