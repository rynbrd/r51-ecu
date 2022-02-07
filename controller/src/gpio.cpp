#include "gpio.h"

// Unshadow Arduino functions.
inline void arduino_pinMode(uint32_t dwPin, uint32_t dwMode) { pinMode(dwPin, dwMode); }
inline int arduino_digitalRead(uint32_t ulPin) { return digitalRead(ulPin); }
inline void arduino_digitalWrite(uint32_t dwPin, uint32_t dwVal) { digitalWrite(dwPin, dwVal); }
inline uint32_t arduino_analogRead(uint32_t ulPin) { return analogRead(ulPin); }
inline void arduino_analogWrite( uint32_t ulPin, uint32_t ulValue ) { analogWrite(ulPin, ulValue); }

class RealGPIO : public GPIO {
    public:
        void pinMode(uint32_t dwPin, uint32_t dwMode) override {
            arduino_pinMode(dwPin, dwMode);
        }

        int digitalRead(uint32_t ulPin) override {
            return arduino_digitalRead(ulPin);
        }

        void digitalWrite(uint32_t dwPin, uint32_t dwVal) override {
            arduino_digitalWrite(dwPin, dwVal);
        }

        uint32_t analogRead(uint32_t ulPin) override {
            return arduino_analogRead(ulPin);
        }

        void analogWrite( uint32_t ulPin, uint32_t ulValue ) override {
            arduino_analogWrite(ulPin, ulValue);
        }
};

RealGPIO real_gpio;

GPIO* GPIO::real() {
    return &real_gpio;
}
