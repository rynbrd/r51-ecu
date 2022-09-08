#ifndef _R51_BRIDGE_CONFIG_H_
#define _R51_BRIDGE_CONFIG_H_

#define SERIAL_DEVICE Serial
#define SERIAL_BAUDRATE 115200
#define SERIAL_WAIT true

// Vehicle CAN bus mode and speed. This is CAN 2.0 at 500K for the R51.
#define VEHICLE_CAN_MODE Canny::CAN20_500K
#define VEHICLE_READ_BUFFER 10
#define VEHICLE_WRITE_BUFFER 2

// Uncomment to enable J1939 on boards that support it.
#define J1939_ENABLE
#define J1939_ADDRESS 0x19
#define J1939_CAN_MODE Canny::CAN20_250K
#define J1939_READ_BUFFER 4
#define J1939_WRITE_BUFFER 2

// Uncomment the following line to enable debug output.
#define DEBUG_ENABLE

// Uncomment to enable Console over serial.
#define CONSOLE_ENABLE

// Uncomment the following line to enable defog heater control.
#define DEFOG_HEATER_ENABLE
#define DEFOG_HEATER_PIN 6
#define DEFOG_HEATER_MS 200

// Uncomment the following line to enable steering keypad.
#define STEERING_KEYPAD_ENABLE
#define STEERING_PIN_A A2
#define STEERING_PIN_B A3

// Uncomment to enable Bluetooth via SPI.
#define BLUETOOTH_ENABLE
#define BLUETOOTH_SPI_CS_PIN 24
#define BLUETOOTH_SPI_IRQ_PIN 23
#define BLUETOOTH_UPDATE_MS 1000

// Uncomment to enable RealDash serial.
#define REALDASH_ENABLE
#define REALDASH_FRAME_ID 0x10
#define REALDASH_HB_ID 0x20
#define REALDASH_HB_MS 500

#endif  // _R51_BRIDGE_CONFIG_H_
