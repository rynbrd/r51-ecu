#include <Arduino.h>
#include <AnalogMultiButton.h>

#define SW_A_PIN A1
#define SW_B_PIN A2
#define SW_PRINT false
#define RDEF_PIN 24
#define RDEF_TIME 300

// Values for a 2k pullup resistor.
static constexpr const int sw_values[] = {20, 332, 1016};
static constexpr const int sw_count = 3;

int sw_print_ms = 0;

class DigitalToggle {
    public:
        DigitalToggle(int pin, uint16_t trigger_ms, int32_t cooldown_ms = -1)
            : pin_(pin), triggered_(0), 
              trigger_ms_(trigger_ms), trigger_time_(0),
              cooldown_ms_(cooldown_ms < 0 ? trigger_ms : (uint16_t)cooldown_ms) {
        }

        void begin() {
            high();
        }

        void loop() {
            if (triggered_) {
                if (millis() - trigger_time_ >= trigger_ms_ + cooldown_ms_) {
                    triggered_ = 0;
                } else if (triggered_ < 2 && millis() - trigger_time_ >= trigger_ms_) {
                    triggered_ = 2;
                    high();
                }
            }
        }

        bool toggle() {
            if (triggered_) {
                return false;
            }

            low();
            triggered_ = 1;
            trigger_time_ = millis();
            return true;
        }

    private:
        void low() {
            pinMode(pin_, OUTPUT);
            digitalWrite(pin_, LOW);
            Serial.println("open drain");
        }

        void high() {
            pinMode(pin_, INPUT);
            Serial.println("high impedence");
        }

        int pin_;
        uint8_t triggered_;
        uint16_t trigger_ms_;
        uint32_t trigger_time_;
        uint16_t cooldown_ms_;
};

AnalogMultiButton swA(SW_A_PIN, sw_count, sw_values);
AnalogMultiButton swB(SW_B_PIN, sw_count, sw_values);
DigitalToggle rdef(RDEF_PIN, RDEF_TIME);

void setup() {
    pinMode(SW_A_PIN, INPUT);
    digitalWrite(SW_A_PIN, LOW);
    pinMode(SW_B_PIN, INPUT);
    digitalWrite(SW_B_PIN, LOW);

    delay(10);
    analogRead(SW_A_PIN);
    analogRead(SW_B_PIN);
    rdef.begin();

    Serial.begin(115200);
    while(!Serial) {
        delay(100);
    }
    Serial.println("boot boot");
}

void loop() {
    if (SW_PRINT) {
        if (sw_print_ms == 0 || millis() - sw_print_ms >= 1000) {
            Serial.print(analogRead(SW_A_PIN));
            Serial.print(", ");
            Serial.println(analogRead(SW_B_PIN));
            sw_print_ms = millis();
        }
    } else {
        swA.update();
        if (swA.onPress(0))  {
            Serial.println("press POWER");
        } else if (swA.onPress(1)) {
            Serial.println("press SEEK_UP");
        } else if (swA.onPress(2)) {
            Serial.println("press VOLUME_UP");
        } else if (swA.onRelease(0))  {
            Serial.println("release POWER");
        } else if (swA.onRelease(1)) {
            Serial.println("release SEEK_UP");
        } else if (swA.onRelease(2)) {
            Serial.println("release VOLUME_UP");
        }

        swB.update();
        if (swB.onPress(0))  {
            Serial.println("press MODE");
        } else if (swB.onPress(1)) {
            Serial.println("press SEEK_DOWN");
        } else if (swB.onPress(2)) {
            Serial.println("press VOLUME_DOWN");
        } else if (swB.onRelease(0))  {
            Serial.println("release MODE");
        } else if (swB.onRelease(1)) {
            Serial.println("release SEEK_DOWN");
        } else if (swB.onRelease(2)) {
            Serial.println("release VOLUME_DOWN");
        }
    }

    rdef.loop();
    while (Serial.available()) {
        if (Serial.read() == '\n') {
            if (rdef.toggle()) {
                Serial.println("toggle rear defrost");
            } else {
                Serial.println("toggle failed, currently active");
            }
        }
    }

    delay(10);
}
