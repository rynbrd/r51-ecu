#include <Arduino.h>
#include <AUnit.h>

#include "test_bus.h"
#include "test_momentary_output.h"
#include "test_realdash.h"
#include "test_steering.h"

using namespace aunit;

void setup() {
    Serial.begin(115200);
    while(!Serial);
    Printer::setPrinter(&Serial);
}

void loop() {
    TestRunner::run();
    delay(1);
}
