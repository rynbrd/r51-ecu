#include "BLE.h"

#include <Arduino.h>
#include <Adafruit_BluefruitLE_SPI.h>

namespace R51 {

bool BLE::begin() {
    if (!bluefruit_.begin()) {
        return false;
    }
    bluefruit_.setMode(BLUEFRUIT_MODE_COMMAND);
    bluefruit_.echo(false);
    bluefruit_.enableModeSwitchCommand(false);
    //TODO: Add debug logging for command failures.
    bluefruit_.atcommand("AT+GAPSTARTADV");
    bluefruit_.setMode(BLUEFRUIT_MODE_DATA);
    return true;
}

bool BLE::connected() {
    return bluefruit_.isConnected();
}

void BLE::disconnect() {
    bluefruit_.atcommand("AT+GAPDISCONNECT");
}

void BLE::forget() {
    //TODO: Add debug logging for command failures.
    bluefruit_.setMode(BLUEFRUIT_MODE_COMMAND);
    bluefruit_.atcommand("AT+GAPDISCONNECT");
    bluefruit_.atcommand("AT+GAPDELBONDS");
    bluefruit_.setMode(BLUEFRUIT_MODE_DATA);
}

void BLE::setName(const uint8_t* name, size_t size) {
    bluefruit_.atcommand("AT+GAPDEVNAME", name, size);
}

void BLE::setName(const char name[]) {
    bluefruit_.atcommand("AT+GAPDEVNAME", name);
}

void BLE::update(uint32_t period_ms) {
    bluefruit_.update(period_ms);
}

void BLE::setOnConnect(void (*cb)(void*), void* arg) {
    bluefruit_.setConnectCallback(cb, arg);
}

void BLE::setOnDisconnect(void (*cb)(void*), void* arg) {
    bluefruit_.setDisconnectCallback(cb, arg);
}

int BLE::available() {
    return bluefruit_.available();
}

int BLE::read() {
    return bluefruit_.read();
}

int BLE::peek() {
    return bluefruit_.peek();
}

size_t BLE::write(uint8_t b) {
    return bluefruit_.write(b);
}

size_t BLE::write(const uint8_t *buffer, size_t size) {
    return bluefruit_.write(buffer, size);
}

}  // namespace R51
