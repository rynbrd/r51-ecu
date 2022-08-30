# See https://github.com/bxparks/EpoxyDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

APP_NAME := ipdm
ARDUINO_LIBS := AUnit AnalogMultiButton CRC32 Canny Caster Common Faker Test Vehicle
EXTRA_CXXFLAGS += -g
include ../../../EpoxyDuino/EpoxyDuino.mk

test: all
	@./$(APP_NAME).out

valgrind: all
	@valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./$(APP_NAME).out
