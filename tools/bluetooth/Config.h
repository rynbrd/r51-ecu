#ifndef _R51_TOOLS_BLUETOOTH_CONFIG_H_
#define _R51_TOOLS_BLUETOOTH_CONFIG_H_

#define MULTICORE

// Serial configuration for the debug console.
#define SERIAL_DEVICE Serial
#define SERIAL_WAIT true

// Multicore buffer settings.
#define IO_CORE_BUFFER_SIZE 2
#define PROC_CORE_BUFFER_SIZE 2

// CAN hardware configuration.
#define MCP2515_CS_PIN 9
#define MCP2518_CS_PIN 12

// Bluetooth configuration.
#define BLUETOOTH_SPI_CS_PIN 22
#define BLUETOOTH_SPI_IRQ_PIN 23
#define BLUETOOTH_UPDATE_MS 1000
#define BLUETOOTH_DEVICE_NAME "R51 Test"

// Realdash enabled with Bluetooth.
#define REALDASH_FRAME_ID 0x10
#define REALDASH_HB_ID 0x20
#define REALDASH_HB_MS 500

#endif  // _R51_TOOLS_BLUETOOTH_CONFIG_H_
