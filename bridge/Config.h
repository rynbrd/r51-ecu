#ifndef _R51_BRIDGE_CONFIG_
#define _R51_BRIDGE_CONFIG_

// Uncomment the following line to enable debug output.
#define DEBUG_ENABLE
#define DEBUG_SERIAL Serial
#define DEBUG_BAUDRATE 115200

// Uncomment the following line to enable defog heater control.
#define DEFOG_HEATER_ENABLE
#define DEFOG_HEATER_PIN 6
#define DEFOG_HEATER_MS 200

// Uncomment the following line to enable steering keypad.
#define STEERING_KEYPAD_ENABLE
#define STEERING_PIN_A A2
#define STEERING_PIN_B A3

#endif  // _R51_BRIDGE_CONFIG_
