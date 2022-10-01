#include <Arduino.h>

#include <Adafruit_seesaw.h>
#include <seesaw_neopixel.h>

#define SS_ADDR 0x36
#define SS_INT_PIN 8
#define SS_INTERRUPT

class RotaryEncoder {
    public:
        RotaryEncoder(TwoWire* wire) : encoder_(wire),
                neopixel_(1, 6,NEO_GRB + NEO_KHZ800, wire), pos_(0), sw_(false),
                pause_int_cb_(nullptr), resume_int_cb_(nullptr), int_cb_arg_(nullptr) {}

        bool begin(uint8_t addr) {
            if (!encoder_.begin(addr) || !neopixel_.begin(addr)) {
                return false;
            }
            encoder_.pinMode(kSwitchPin, INPUT_PULLUP);
            delay(10);
            encoder_.setGPIOInterrupts((uint32_t)1 << kSwitchPin, 1);
            encoder_.enableEncoderInterrupt();
            update();
            return true;
        }

        void update() {
            if (pause_int_cb_ != nullptr) {
                pause_int_cb_(int_cb_arg_);
            }
            pos_ = encoder_.getEncoderPosition();
            sw_ = !encoder_.digitalRead(kSwitchPin);
            if (resume_int_cb_ != nullptr) {
                resume_int_cb_(int_cb_arg_);
            }
        }

        int32_t getPosition() { return pos_; }
        bool getSwitch() { return sw_; }

        void setColor(uint8_t r, uint8_t g, uint8_t b) {
            if (pause_int_cb_ != nullptr) {
                pause_int_cb_(int_cb_arg_);
            }
            neopixel_.setPixelColor(0, neopixel_.Color(r, g, b));
            neopixel_.show();
            if (resume_int_cb_ != nullptr) {
                resume_int_cb_(int_cb_arg_);
            }
        }

        void setInterruptCallbacks(void (*pause_cb)(void*),
                void (*resume_cb)(void*), void* arg = nullptr) {
            pause_int_cb_ = pause_cb;
            resume_int_cb_ = resume_cb;
            int_cb_arg_ =  arg;
        }

    private:
        Adafruit_seesaw encoder_;
        seesaw_NeoPixel neopixel_;

        int32_t pos_;
        bool sw_;

        void (*pause_int_cb_)(void*);
        void (*resume_int_cb_)(void*);
        void* int_cb_arg_;

        static const int kSwitchPin = 24;
};

RotaryEncoder encoder(&Wire);

bool read;
int pos;
int new_pos;
bool press;
bool new_press;
uint8_t n = 0; 
uint8_t rgb[] = {20, 20, 20};

void printByte(uint8_t b) {
    if (b <= 0x0F) {
        Serial.print("0");
    }
    Serial.print(b, HEX);
}

void printColor() {
    Serial.print("color: ");
    for (uint8_t i = 0; i < 3; ++i) {
        printByte(rgb[i]);
    }
    Serial.print(" (");
    for (uint8_t i = 0; i < 3; ++i) {
        Serial.print(rgb[i]);
        if (i != 3) {
            Serial.print(", ");
        }
    }
    Serial.println(")");
}

#if defined(SS_INTERRUPT)
void encoderInterrupt() {
    read = true;
    Serial.println("*");
}

void attachInterrupts(void*) {
    attachInterrupt(digitalPinToInterrupt(SS_INT_PIN), encoderInterrupt, FALLING);
}

void detachInterrupts(void*) {
    detachInterrupt(digitalPinToInterrupt(SS_INT_PIN));
}
#endif

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }

    // connect to encoder
    Serial.print("init encoder ");
    if (!encoder.begin(SS_ADDR)) {
        Serial.println();
        Serial.print("encoder not found at 0x");
        Serial.println(SS_ADDR, HEX);
        while(true) {
            delay(1000);
        }
    }

    // get initial state
    Serial.print(". ");
    pos = encoder.getPosition();
    new_pos = pos;
    press = encoder.getSwitch();
    new_press = press;

    // attach interrupt callback
    Serial.print(". ");
#if defined(SS_INTERRUPT)
    read = false;
    attachInterrupts(nullptr);
    encoder.setInterruptCallbacks(detachInterrupts, attachInterrupts);
#else
    read = true;
#endif

    // turn on the light, not too bright
    Serial.print(". ");
    encoder.setColor(30, 30, 30);

    Serial.println("ready");
}

void loop() {
#if defined(SS_INTERRUPT)
    if (read) {
        encoder.update();
    }
#else
    encoder.update();
#endif

    new_pos = encoder.getPosition();
    if (new_pos != pos) {
        if (new_pos < pos) {
            rgb[n]++;
        } else {
            rgb[n]--;
        }
        pos = new_pos;
        printColor();
        encoder.setColor(rgb[0], rgb[1], rgb[2]);
    }

    new_press = encoder.getSwitch();
    if (new_press != press) {
        press = new_press;
        if (!press) {
            ++n;
            if (n > 2) {
                n = 0;
            }
            if (n == 0) {
                Serial.println("r");
            } else if (n == 1) {
                Serial.println("g");
            } else if (n == 2) {
                Serial.println("b");
            }
        }
    }

    delay(10);
}
