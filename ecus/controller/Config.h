#ifndef _R51_CTRL_CONFIG_H_
#define _R51_CTRL_CONFIG_H_

// Uncomment the following line to enable debug output.
#define DEBUG_ENABLE
#define CONSOLE_ENABLE

// Serial configuration for the debug console.
#define SERIAL_DEVICE Serial
#define SERIAL_BAUDRATE 115200
#define SERIAL_WAIT true

// I2C hardware configuration.
#define I2C_DEVICE Wire1
#define I2C_SDA_PIN 6
#define I2C_SCL_PIN 7

// CAN hardware configuration.
#define MCP2515_CS_PIN 9
#define MCP2515_INT_PIN 11

// Multicore buffer settings.
#define IO_CORE_BUFFER_SIZE 32
#define PROC_CORE_BUFFER_SIZE 16

#define WATCHDOG_TIMEOUT 500

// J1939 gateway configuration.
#define J1939_ADDRESS 0x19
#define J1939_NAME 0x00000BB000FFFAC0
#define J1939_PROMISCUOUS true
#define J1939_CAN_MODE Canny::CAN20_250K
#define J1939_READ_BUFFER 128
#define J1939_WRITE_BUFFER 4

// Rotary encoder configuration.
#define ROTARY_ENCODER_ID 0x01
#define ROTARY_ENCODER_INTR_PIN 8
#define ROTARY_ENCODER_ADDR0 0x36
#define ROTARY_ENCODER_ADDR1 0x37

// HMI display configuration.
#define HMI_DEVICE Serial1
#define HMI_BAUDRATE 115200

// External J1939 keypad configuration.
#define BLINK_KEYPAD_ID 0x02
#define BLINK_KEYPAD_ADDR 0x24
#define BLINK_KEYPAD_KEYS 8

// External J1939 PDU configuration.
#define BLINK_KEYBOX_ID 0x01
#define BLINK_KEYBOX_ADDR 0x23

// J1939 address of the bridge ECU.
#define BRIDGE_ADDRESS 0x18

// Keypad ID of the steering module.
#define STEERING_KEYPAD_ID 0x00

#endif  // _R51_CTRL_CONFIG_H_
