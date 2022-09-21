#include <Arduino.h>
#include <Nextion.h>

int n;
uint8_t buffer[256];
Nextion::Protocol hmi(&Serial1);

void setup() {
    Serial.begin(115200);
    Serial1.begin(512000);
}

void loop() {
    n = Serial.readBytesUntil('\n', buffer, 256);
    if (n > 0) {
        for (int i = 0; i < n; i++) {
            Serial.print((char)buffer[i]);
        }
        Serial.println();
        hmi.send(buffer, n);
    }
    n = hmi.recv(buffer, 246);
    if (n > 0) {
        for (int i = 0; i < n; i++) {
            if (buffer[i] < 0x0F) {
                Serial.print("0");
            }
            Serial.print(buffer[i], HEX);
        }
        Serial.println();
    }
}
