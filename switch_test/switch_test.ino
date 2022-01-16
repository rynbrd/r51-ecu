#include <Arduino.h>
#include <AnalogMultiButton.h>

#define SW_A_PIN A2
#define SW_B_PIN A3
#define RDEF_PIN 6

static constexpr const int sw_count = 3;
static constexpr const int sw_values[sw_count] = {50, 280, 640};
AnalogMultiButton swA(SW_A_PIN, sw_count, sw_values);
AnalogMultiButton swB(SW_B_PIN, sw_count, sw_values);

bool rdef = false;

void toggleRdef() {
    rdef = !rdef;
    digitalWrite(RDEF_PIN, rdef);
    if (rdef) {
        Serial.println("turn rdef on");
    } else {
        Serial.println("turn rdef off");
    }
}

void setup() {
    pinMode(RDEF_PIN, OUTPUT);
    digitalWrite(RDEF_PIN, 0);
    rdef = false;

    Serial.begin(115200);
    while(!Serial) {
        delay(100);
    }
}

void loop() {
    swA.update();
    if (swA.onPress(0))  {
        Serial.println("press POWER");
    } else if (swA.onPress(1)) {
        Serial.println("press SEEK_DOWN");
    } else if (swA.onPress(2)) {
        Serial.println("press VOLUME_DOWN");
    } else if (swA.onRelease(0))  {
        Serial.println("release POWER");
    } else if (swA.onRelease(1)) {
        Serial.println("release SEEK_DOWN");
    } else if (swA.onRelease(2)) {
        Serial.println("release VOLUME_DOWN");
    }

    swB.update();
    if (swB.onPress(0))  {
        Serial.println("press MODE");
    } else if (swB.onPress(1)) {
        Serial.println("press SEEK_UP");
    } else if (swB.onPress(2)) {
        Serial.println("press VOLUME_UP");
    } else if (swB.onRelease(0))  {
        Serial.println("release MODE");
    } else if (swB.onRelease(1)) {
        Serial.println("release SEEK_UP");
    } else if (swB.onRelease(2)) {
        Serial.println("release VOLUME_UP");
    }

    while (Serial.available()) {
        if (Serial.read() == '\n') {
            toggleRdef();
        }
    }

    delay(10);
}
