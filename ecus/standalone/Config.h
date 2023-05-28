#ifndef _R51_STANDALONE_CONFIG_H_
#define _R51_STANDALONE_CONFIG_H_

// Uncomment the following line to enable debug output on serial and/or the
// interactive console.
#define DEBUG_ENABLE
#define CONSOLE_ENABLE
#define CONSOLE_INITIAL_MUTE true

// Serial configuration for the debug console.
#define SERIAL_DEVICE Serial
#define SERIAL_BAUDRATE 115200
#define SERIAL_WAIT false

// Arduino board constants.
#define ARDUINO_ANALOG_RESOLUTION 4096

// I2C hardware configuration.
#define I2C_DEVICE Wire1
#define I2C_SDA_PIN 6
#define I2C_SCL_PIN 7

// CAN hardware configuration.
#define MCP2515_CS_PIN 9
#define MCP2515_IRQ_PIN 11
#define MCP2518_CS_PIN 12
#define MCP2518_IRQ_PIN 13

// Multicore buffer settings.
#define IO_CORE_BUFFER_SIZE 256
#define PROC_CORE_BUFFER_SIZE 16

// Vehicle CAN bus mode and speed. This is CAN 2.0 at 500K for the R51.
#define VEHICLE_CAN_MODE Canny::CAN20_500K
#define VEHICLE_PROMISCUOUS false
#define VEHICLE_READ_BUFFER 16
#define VEHICLE_WRITE_BUFFER 2

// J1939 gateway configuration.
#define J1939_ADDRESS 0x20
#define J1939_NAME 0x00001BB000FFFAC0
#define J1939_PROMISCUOUS false
#define J1939_CAN_MODE Canny::CAN20_250K
#define J1939_READ_BUFFER 128
#define J1939_WRITE_BUFFER 4

// Defrost heater configuration.
#define DEFROST_HEATER_PIN 24
#define DEFROST_HEATER_MS 300

// Steering keypad configuration.
#define STEERING_KEYPAD_ID 0x00
#define STEERING_KEYPAD_ENABLE
#define STEERING_PIN_A A1
#define STEERING_PIN_B A2
#define STEERING_DEBOUNCE_MS 20

// Uncomment to enable Bluetooth via SPI.
#define BLUETOOTH_ENABLE
#define BLUETOOTH_DEVICE_NAME "R51 Controls"
#define BLUETOOTH_SPI_CS_PIN 22
#define BLUETOOTH_SPI_IRQ_PIN 23
#define BLUETOOTH_UPDATE_MS 1000

// Realdash enabled with Bluetooth.
#define REALDASH_FRAME_ID 0x10
#define REALDASH_HB_ID 0x20
#define REALDASH_HB_MS 500

// Rotary encoder configuration.
#define ROTARY_ENCODER_ID 0x01
#define ROTARY_ENCODER_IRQ_PIN 21
#define ROTARY_ENCODER_ADDR0 0x36
#define ROTARY_ENCODER_ADDR1 0x37

// HMI display configuration.
#define HMI_DEVICE Serial1
#define HMI_BAUDRATE 512000

// External J1939 keypad configuration.
#define BLINK_KEYPAD_ID 0x02
#define BLINK_KEYPAD_ADDR 0x24
#define BLINK_KEYPAD_KEYS 8

// External J1939 PDU configuration.
#define BLINK_KEYBOX_ID 0x01
#define BLINK_KEYBOX_ADDR 0x23

#endif  // _R51_STANDALONE_CONFIG_H_
