#ifndef _R51_BLUETOOTH_BLE_H_
#define _R51_BLUETOOTH_BLE_H_

#include <Arduino.h>
#include <Adafruit_BluefruitLE_SPI.h>

namespace R51 {

// Create a BLE stream connection for serial communication.
class BLE : public Stream {
    public:
        // Create a BLE object that connects to a Bluefruit SPI module with the
        // given CS and IRQ pins.
        BLE(int8_t spi_cs_pin, int8_t spi_irq_pin) :
            bluefruit_(spi_cs_pin, spi_irq_pin, -1) {}

        // Initialize the Bluefruit device. Configure the Bluefruit device for
        // serial communication and disable advertising (pairing mode). Return
        // true on success.
        bool begin();

        // Return true if connected to a host.
        bool connected();

        // Disconnect the currently connected device and begin advertising.
        // Advertising stops after a connection is established.
        void disconnect();

        // Disconnect and forget any stored hosts. 
        void forget();

        // Set the name of the device. This is persisted across reboots.
        void setName(const uint8_t* name, size_t size);

        // Should be called periodically to retrieve BLE device status. If IRQ
        // is wired then this should be called with period_ms = 0 on interrupt.
        void update(uint32_t period_ms = 200);

        // Set the callback to execute when a host device connects.
        void setOnConnect(void (*cb)(void));

        // Set the callback to execute when a host device disconnects.
        void setOnDisconnect(void (*cb)(void));

        // Stream functions.
        int available() override;
        int read() override;
        int peek() override;
        size_t write(uint8_t) override;
        size_t write(const uint8_t* buffer, size_t size) override;

    private:

        Adafruit_BluefruitLE_SPI bluefruit_;
};

}  // namespace R51

#endif  // _R51_BLUETOOTH_BLE_H_
