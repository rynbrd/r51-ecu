# See https://github.com/bxparks/EpoxyDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

APP_NAME := tests
ARDUINO_LIBS := AnalogMultiButton ArduinoCanny ArduinoCaster ArduinoFaker AUnit CRC32
EXTRA_CPPFLAGS += -Isrc
EXTRA_CXXFLAGS += -g
include ../../libraries/EpoxyDuino/EpoxyDuino.mk

test: all
	@./$(APP_NAME).out

valgrind: all
	@valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes --track-origins=yes ./tests.out
