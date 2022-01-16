#include <Arduino.h>
#include <AnalogMultiButton.h>

#define SW_A_PIN A2
#define SW_B_PIN A3
#define RDEF_PIN 6

static constexpr const int sw_count = 3;
static constexpr const int sw_values[sw_count] = {50, 280, 640};

class DigitalToggle {
    public:
        DigitalToggle(int pin, uint16_t trigger_ms, int32_t cooldown_ms = -1, bool high = true)
            : pin_(pin), high_(high), triggered_(false), 
              trigger_ms_(trigger_ms), trigger_time_(0),
              cooldown_ms_(cooldown_ms < 0 ? trigger_ms : (uint16_t)cooldown_ms) {}

        void Loop() {
            if (triggered_) {
                if (millis() - trigger_time_ >= trigger_ms_ + cooldown_ms_) {
                    triggered_ = false;
                } else if (millis() - trigger_time_ >= trigger_ms_) {
                    digitalWrite(pin_, !high_);
                }
            }
        }

        bool Toggle() {
            if (triggered_) {
                return false;
            }

            digitalWrite(pin_, high_);
            triggered_ = true;
            trigger_time_ = millis();
            return true;
        }

    private:
        int pin_;
        bool high_;
        bool triggered_;
        uint16_t trigger_ms_;
        uint32_t trigger_time_;
        uint16_t cooldown_ms_;
};

AnalogMultiButton swA(SW_A_PIN, sw_count, sw_values);
AnalogMultiButton swB(SW_B_PIN, sw_count, sw_values);
DigitalToggle rdef(RDEF_PIN, 200);

void setup() {
    pinMode(RDEF_PIN, OUTPUT);
    digitalWrite(RDEF_PIN, 0);

    Serial.begin(115200);
    while(!Serial) {
        delay(100);
    }
    Serial.println("Hello, world.");
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

    rdef.Loop();
    while (Serial.available()) {
        if (Serial.read() == '\n') {
            if (rdef.Toggle()) {
                Serial.println("toggle rear defrost");
            } else {
                Serial.println("toggle failed, currently active");
            }
        }
    }

    delay(10);
}
