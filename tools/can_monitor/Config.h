#ifndef _R51_TOOLS_CAN_MONITOR_CONFIG_H_
#define _R51_TOOLS_CAN_MONITOR_CONFIG_H_

// Serial configuration for the debug console.
#define SERIAL_DEVICE Serial
#define SERIAL_BAUDRATE 115200
#define SERIAL_WAIT false

// CAN hardware configuration.
#define MCP2515_CS_PIN 9
#define MCP2515_IRQ_PIN 11
#define MCP2518_CS_PIN 12
#define MCP2518_IRQ_PIN 13

// CAN bus configuration.
#define CAN_READ_BUFFER 32
#define CAN_WRITE_BUFFER 2
#define CAN_MODE Canny::CAN20_500K  // 500k for vehicle, 250k for J1939

#endif  // _R51_TOOLS_CAN_MONITOR_CONFIG_H_
