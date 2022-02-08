#ifndef __R51_TESTS_MOCK_GPIO__
#define __R51_TESTS_MOCK_GPIO__

#include "src/gpio.h"


class MockGPIO : public GPIO {
    public:
        MockGPIO(uint32_t pin_count = 64) {
            pin_values_ = new uint32_t[pin_count]();
            pin_modes_ = new uint32_t[pin_count]();
        }

        ~MockGPIO() {
            delete pin_values_;
            delete pin_modes_;
        }


        // Set the pin mode.
        void pinMode(uint32_t dwPin, uint32_t dwMode) override {
            pin_modes_[dwPin] = dwMode;
        }

        // Get the set mode for a pin.
        uint32_t pinMode(uint32_t dwPin) {
            return pin_modes_[dwPin]; 
        }

        // Read a digital pin value.
        int digitalRead(uint32_t ulPin) override {
            return pin_values_[ulPin];
        }

        // Write a digital pin value.
        void digitalWrite(uint32_t dwPin, uint32_t dwVal) override {
            pin_values_[dwPin] =  dwVal;
        }

        // Read an analog pin value.
        uint32_t analogRead(uint32_t ulPin) override {
            return pin_values_[ulPin];
        }

        // Write an analog pin value.
        void analogWrite(uint32_t ulPin, uint32_t ulValue) override {
            pin_values_[ulPin] = ulValue;
        }
    private:
        uint32_t* pin_values_;
        uint32_t* pin_modes_;
};

#endif  // __R51_TESTS_MOCK_GPIO__
