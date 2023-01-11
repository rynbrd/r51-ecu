#ifndef _R51_TOOLS_VEHICLE_CONFIG_H_
#define _R51_TOOLS_VEHICLE_CONFIG_H_

// Serial configuration for the debug console.
#define SERIAL_DEVICE Serial
#define SERIAL_BAUDRATE 115200
#define SERIAL_WAIT true

// CAN hardware configuration.
#define MCP2515_CS_PIN 9
#define MCP2515_INT_PIN 11

// Vehicle CAN bus mode and speed. This is CAN 2.0 at 500K for the R51.
#define VEHICLE_CAN_MODE Canny::CAN20_500K
#define VEHICLE_PROMISCUOUS false
#define VEHICLE_READ_BUFFER 16
#define VEHICLE_WRITE_BUFFER 2

// Defrost heater configuration.
#define DEFROST_HEATER_PIN 6
#define DEFROST_HEATER_MS 200

// Steering keypad configuration.
#define STEERING_KEYPAD_ID 0x00
#define STEERING_KEYPAD_ENABLE
#define STEERING_PIN_A A1
#define STEERING_PIN_B A2

#endif  // _R51_TOOLS_VEHICLE_CONFIG_H_
