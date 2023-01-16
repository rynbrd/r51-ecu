#ifndef _R51_TOOLS_ROTARY_ENCODER_CONFIG_H_
#define _R51_TOOLS_ROTARY_ENCODER_CONFIG_H_

// Serial configuration for the debug console.
#define SERIAL_DEVICE Serial
#define SERIAL_BAUDRATE 115200
#define SERIAL_WAIT true

// I2C device configuration.
#define I2C_DEVICE Wire1
#define I2C_SDA_PIN 6
#define I2C_SCL_PIN 7

// Rotary encoder configuration.
#define ROTARY_ENCODER_ID 0x01
#define ROTARY_ENCODER_IRQ_PIN 21
#define ROTARY_ENCODER_ADDR0 0x36
#define ROTARY_ENCODER_ADDR1 0x37

#endif  // _R51_TOOLS_ROTARY_ENCODER_CONFIG_H_
