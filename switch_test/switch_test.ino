#include <Arduino.h>
#include <AnalogMultiButton.h>

class SteeringSwitch {
    public:
        enum Button {
            POWER,
            MODE,
            SEEK_UP,
            SEEK_DOWN,
            VOLUME_UP,
            VOLUME_DOWN,
        };

        SteeringSwitch(int pin_a, int pin_b) :
            sw_a_(pin_a, sizeof(sw_values_) / sizeof(sw_values_[0]), sw_values_),
            sw_b_(pin_b, sizeof(sw_values_) / sizeof(sw_values_[0]), sw_values_) {}

        void loop() {
            sw_a_.update();
            sw_b_.update();
        }

        bool onPress(Button button) {
            switch (button) {
                case POWER:
                    return sw_a_.onPress(0);
                case MODE:
                    return sw_b_.onPress(0);
                case SEEK_UP:
                    return sw_a_.onPress(1);
                case SEEK_DOWN:
                    return sw_b_.onPress(1);
                case VOLUME_UP:
                    return sw_a_.onPress(2);
                case VOLUME_DOWN:
                    return sw_b_.onPress(2);
                default:
                    break;
            }
            return false;
        }

        bool onRelease(Button button) {
            switch (button) {
                case POWER:
                    return sw_a_.onRelease(0);
                case MODE:
                    return sw_b_.onRelease(0);
                case SEEK_UP:
                    return sw_a_.onRelease(1);
                case SEEK_DOWN:
                    return sw_b_.onRelease(1);
                case VOLUME_UP:
                    return sw_a_.onRelease(2);
                case VOLUME_DOWN:
                    return sw_b_.onRelease(2);
                default:
                    break;
            }
            return false;
        }

    private:
        AnalogMultiButton sw_a_;
        AnalogMultiButton sw_b_;
        static const int sw_values_[3] = {0, 210, 416};
};

void PrintButtonName(SteeringSwitch::Button button) {
    switch (button) {
        case SteeringSwitch::POWER:
            Serial.print("POWER");
            break;
        case SteeringSwitch::MODE:
            Serial.print("MODE");
            break;
        case SteeringSwitch::SEEK_UP:
            Serial.print("SEEK_UP");
            break;
        case SteeringSwitch::SEEK_DOWN:
            Serial.print("SEEK_DOWN");
            break;
        case SteeringSwitch::VOLUME_UP:
            Serial.print("VOLUME_UP");
            break;
        case SteeringSwitch::VOLUME_DOWN:
            Serial.print("VOLUME_DOWN");
            break;
        default:
            break;
    }
}

void PrintButtons(const SteeringSwitch& sw) {
    for (int i = SteeringSwitch::POWER; i < SteeringSwitch::VOLUME_DOWN; i++) {
        if (sw.onPress(i)) {
            Serial.print("press ");
            PrintButtonName(i);
            Serial.println("");
        }
        if (sw.onRelease(i)) {
            Serial.print("release ");
            PrintButtonName(i);
            Serial.println("");
        }
    }
}

void setup() {
    Serial.begin(115200);
    while(!Serial) {
        delay(100);
    }
}

void loop() {
    Serial.println(analogRead(A0));
    delay(500);
}
