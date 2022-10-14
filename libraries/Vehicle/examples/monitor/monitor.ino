// Examples script which monitors vehicle state.

#include <Canny.h>
#include <Canny/Detect.h>
#include <NissanR51.h>

using namespace NissanR51;

ClimateSystemState climate_system;
ClimateTemperatureState climate_temp;
ECMCoolantState coolant;
IPDMState ipdm;
TirePressure tires;

Canny::Error err;
Canny::Frame frame(64);

void printClimate();
void printEcm();
void printIpdm();
void printTires();

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(100); }
    if (!CAN.begin(Canny::CAN20_500K)) {
        Serial.println("failed to setup CAN");
        while (true) { delay(1000); }
    }
    Serial.println("monitoring Nissan R51 Pathfinder");
}

void loop() {
    if (CAN.read(&frame) != Canny::ERR_OK) {
        return;
    }

    if (climate_system.handle(frame) | climate_temp.handle(frame)) {
        printClimate();
    }
    if (coolant.handle(frame)) {
        printEcm();
    }
    if (ipdm.handle(frame)) {
        printIpdm();
    }
    if (tires.handle(frame)) {
        printTires();
    }
}

void printClimate() {
    // "climate: off/auto/manual (driver | passenger) fan feet/face/wind/recirc
    Serial.print("climate: ");
    switch (climate_system.system()) {
        case CLIMATE_SYSTEM_OFF:
            Serial.print("off");
            break;
        case CLIMATE_SYSTEM_AUTO:
            Serial.print("aut");
            break;
        case CLIMATE_SYSTEM_MANUAL:
            Serial.print("man");
            break;
        case CLIMATE_SYSTEM_DEFOG:
            Serial.print("def");
            break;
    }
    Serial.print(" ( ");
    if (climate_temp.driver_temp() < 10) {
        Serial.print(" ");
    }
    Serial.print(climate_temp.driver_temp());
    Serial.print(" / ");
    if (climate_temp.passenger_temp() < 10) {
        Serial.print(" ");
    }
    Serial.print(climate_temp.passenger_temp());
    if (climate_temp.units() == UNITS_METRIC) {
        Serial.print("C");
    } else {
        Serial.print("F");
    }
    Serial.print(" ) ");
    switch (climate_system.vents()) {
        case CLIMATE_VENTS_CLOSED:
            Serial.print("***");
            break;
        case CLIMATE_VENTS_FACE:
            Serial.print("f**");
            break;
        case CLIMATE_VENTS_FACE_FEET:
            Serial.print("ff*");
            break;
        case CLIMATE_VENTS_FEET:
            Serial.print("*f*");
            break;
        case CLIMATE_VENTS_FEET_WINDSHIELD:
            Serial.print("*fw");
            break;
        case CLIMATE_VENTS_WINDSHIELD:
            Serial.print("**w");
            break;
    }
    if (climate_system.recirculate()) {
        Serial.print("r");
    } else {
        Serial.print("*");
    }
    Serial.print(" ( ");
    if (climate_temp.outside_temp() < 10) {
        Serial.print(" ");
    }
    Serial.print(climate_temp.outside_temp());
    if (climate_temp.units() == UNITS_METRIC) {
        Serial.print("C");
    } else {
        Serial.print("F");
    }
    Serial.println(" )");
}

void printEcm() {
    Serial.print("engine temps: ");
    if (coolant.coolant_temp() < 10) {
        Serial.print("  ");
    } else if (coolant.coolant_temp() < 100) {
        Serial.print(" ");
    }
    Serial.print(coolant.coolant_temp());
    Serial.println("C");
}

void printOnOff(bool val) {
    Serial.print(val ? "on " : "off");
}

void printIpdm() {
    Serial.print("ipdm: headlamp=");
    if (ipdm.high_beams()) {
        Serial.print("high");
    } else if (ipdm.low_beams()) {
        Serial.print("low ");
    } else {
        Serial.print("off ");
    }
    Serial.print(" fog=");
    printOnOff(ipdm.fog_lights());
    Serial.print(" running=");
    printOnOff(ipdm.running_lights());
    Serial.print(" defrost=");
    printOnOff(ipdm.defrost());
    Serial.print(" ac=");
    printOnOff(ipdm.ac_compressor());
    Serial.println();
}

void printDouble(double val, unsigned int precision) {
    Serial.print(int(val));
    Serial.print(".");
    unsigned int frac;
    if (val >= 0) {
      frac = (val - int(val)) * precision;
    } else {
       frac = (int(val) - val) * precision;
    }
    int tmp = frac;
    while (tmp /= 10) {
        precision /= 10;
    }
    precision /= 10;
    while (precision /= 10) {
        Serial.print("0");
    }
    Serial.println(frac, DEC) ;
}

void printTirePressure(double val) {
    if (val < 0) {
        Serial.print("  *  ");
        return;
    }
    if (val < 10) {
        Serial.print(" ");
    }
    printDouble(val, 2);
}

void printTires() {
    Serial.print("tires: ");
    printTirePressure(tires.tire_pressure_1());
    printTirePressure(tires.tire_pressure_2());
    printTirePressure(tires.tire_pressure_3());
    printTirePressure(tires.tire_pressure_4());
}
