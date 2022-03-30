#ifndef __R51_GPIO__
#define __R51_GPIO__

#include <Arduino.h>


// Base GPIO interface. Allows GPIO interaction to be mocked.
class GPIO {
    public:
        // Return the real GPIO interface.
        static GPIO* real();

        GPIO() = default;
        virtual ~GPIO() = default;

        // Set the pin mode.
        virtual void pinMode(uint32_t dwPin, uint32_t dwMode) = 0;

        // Read a digital pin value.
        virtual int digitalRead(uint32_t ulPin) = 0;

        // Write a digital pin value.
        virtual void digitalWrite(uint32_t dwPin, uint32_t dwVal) = 0;

        // Read an analog pin value.
        virtual uint32_t analogRead(uint32_t ulPin) = 0;

        // Write an analog pin value.
        virtual void analogWrite(uint32_t ulPin, uint32_t ulValue) = 0;
};

#endif  // __R51_GPIO__
