# See https://github.com/bxparks/EpoxyDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

APP_NAME := fusion
ARDUINO_LIBS := AUnit Adafruit_BluefruitLE Adafruit_BusIO Adafruit_Seesaw \
	AnalogMultiButton Blink Bluetooth ByteOrder CRC32 Canny Caster Core \
	Controls Foundation Faker Test Vehicle
EXTRA_CXXFLAGS += -g -fpermissive
include ../../../EpoxyDuino/EpoxyDuino.mk

test: all
	@./$(APP_NAME).out

valgrind: all
	@valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./$(APP_NAME).out
