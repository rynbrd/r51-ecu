# See https://github.com/bxparks/EpoxyDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

APP_NAME := tests
ARDUINO_LIBS := AnalogMultiButton ArduinoCaster ArduinoFaker AUnit CANBed CRC32
include ../../libraries/EpoxyDuino/EpoxyDuino.mk

test: all
	@./$(APP_NAME).out
