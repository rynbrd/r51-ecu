#ifndef _R51_TOOLS_BLUETOOTH_CONFIG_H_
#define _R51_TOOLS_BLUETOOTH_CONFIG_H_

// Serial configuration for the debug console.
#define SERIAL_DEVICE Serial
#define SERIAL_BAUDRATE 115200
#define SERIAL_WAIT true

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
