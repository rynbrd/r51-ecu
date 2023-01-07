# Nissan Pathfinder R51 Systems

This repo contains documentation and code to allow integration of custom ECU's
into the Nissan Pathfinder R51 (2005 - 2012). Some of these integrations may
work with other Nissan vehicles (Armada, QX56, etc) but have not been tested.

These ECU implementations specifically target removal of the stock infotainment
center.

## Supported Environment

This repo targets the [Longan Labs] CANBed boards based on the RP2040. The
[CANBed Dual] board is used to bridge the vehicle bus to J1939 and the [CANBed
RP2040] board for individual components.

This is an Arduino project and uses Earle Philhower's [Arduino Pico Core]. All
libraries are either included directly in the repo or linked in using git
submodules. Libraries are included in the [libraries] directory. The Arduino
IDE should be configured to use this repo's root as the sketchbook location in
order to utilize the included libraries.

### CANBed Dual Notes

There have been multiple versions of the [CANBed Dual] board. The latest
version uses a separate GD32 chip as the CAN controller while the older version
used MCP2515 and MCP2518 chips for CAN 2.0 and CAN FD respectively. This
project uses the older version. Additional work would be needed to support the
GD32 flavor of the board.

## Components

### Bridge ECU

The [bridge](bridge) module serves to bridge connectivity between vehicle
systems, a J1939 bus, and serial over Bluetooth BLE for [RealDash]
connectivity. It connects to the infotainment harness via GPIO to retain the
steering wheel controls and to control the rear window/side mirror defrost
heaters.

Its hardware consists of a [CANBed Dual] board and an Adafruit
[Bluefruit LE SPI Friend] for BLE connectivity.

If you're interested in implementing your own set of controllers over J1939
then this would be immediately usable to you.

### Controller ECU

The [controller](controller) module provides a user interface to replace the
functionality originally part of the stock system. This includes climate
control and settings. It also adds support for the
[third party components](#Third Party Components) used by this implementation.

Its hardware consists of a [CANBed RP2040] board, two Adafruit [I2C QT Rotary Encoder] boards, and a Nextion [NX8048K050].

### Standalone ECU

The [standalone](standalone) module combines the bridge and controller
functionality to reduce hardware BOM.

### Third Party Components

The controller integrates with the following aftermarket J1939 systems:

* [Fusion Apollo MS-WB670](https://www.garmin.com/en-US/p/690864)
* [Blink Marine PKP-2400-SI](https://www.blinkmarine.com/powerkey-pro-can-keypad-2/)
* [Blink Marinc Keybox](https://www.blinkmarine.com/can-bus-relay/)

## Vehicle Details 

The vehicle systems have been reverse engineered to determine CAN bus operation and pinouts. These are documented in the following:

* [Climate Control](docs/climate.md) - Operational details including CAN frame definitions.
* [Settings Management](docs/settings.md) - Operational details on managing vehicle settings.
* [Pinouts](docs/pinouts.md) - Pin outs and connector part numbers for the infotainment unit.

# License

The contents of this repository are covered under the GPLv3. See the [LICENSE]
file for full details.


[Arduino Pico Core]: https://github.com/earlephilhower/arduino-pico
[Longan Labs]: https://www.longan-labs.cc/
[CANBed RP2040]: http://docs.longan-labs.cc/1030018/
[CANBed Dual]: http://docs.longan-labs.cc/1030019/
[Bluefruit LE SPI Friend]: https://learn.adafruit.com/introducing-the-adafruit-bluefruit-spi-breakout
[I2C QT Rotary Encoder]: https://learn.adafruit.com/adafruit-i2c-qt-rotary-encoder
[NX8048K050]: https://www.amazon.com/NEXTION-Enhanced-Display-Raspberry-NX8048K050/dp/B07BL3BTM2/
[RealDash]: http://realdash.net
[LICENSE]: LICENSE
