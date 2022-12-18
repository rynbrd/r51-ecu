# Nissan Pathfinder R51 Systems

This repo contains documentation and code to allow integration of custom ECU's
into the Nissan Pathfinder R51 (2005 - 2012). Some of these integrations may
work with other Nissan vehicles (Armada, QX56, etc) but have not been tested.

## Supported Environment

This repo targets the [Longan Labs] CANBed boards based on the RP2040. The
[CANBed Dual] board is used to bridge the vehicle bus to J1939 and the [CANBed
RP2040] board for individual components.

This is an Arduino project and uses Earle Philhower's [Arduino Pico Core]. All
libraries are either included directly in the repo or linked in using git
submodules. Libraries are included in the [libraries] directory. The Arduino
IDE should be configured to use this repo's root as the sketchbook location in
order to utilize the included libraries.

## Infotainment Center Replacement

The [controller](controller) directory contains an Arduino sketch that
implements the control functionality of the stock infotainment system. This
includes climate control and vehicle settings management. It integrates with
[RealDash] so that this functionality can be controlled from a tablet.
Configuration is available under the [realdash](realdash) directory.

Using this system allows the stock head unit and LCD panel to be replaced by an
aftermarket audio system. The stock head unit must be removed from the car in
order for this system to operate properly.

The software targets the Arduino Lenoardo compatible [CAN Bed M4] but should
work with any SAME51 implementation with some small updates. The MCP based
CANBed controllers do not work as they are too slow.

The software exposes a serial protocol compatible with [RealDash]. The
[realdash directory](./realdash) contains the necessary configuration.


## Vehicle Details 

* [Climate Control](docs/climate.md) - Operational details including CAN frame definitions.
* [Settings Management](docs/settings.md) - Operational details on managing vehicle settings.
* [Pinouts](docs/pinouts.md) - Pin outs and connector part numbers for the infotainment unit.


# License

The contents of this repository are covered under the GPLv3. See the [LICENSE]
file for full details.


[Longan Labs]: https://www.longan-labs.cc/
[CANBed RP2040]: http://docs.longan-labs.cc/1030018/
[CANBed Dual]: http://docs.longan-labs.cc/1030019/
[arduino-pico]: https://github.com/earlephilhower/arduino-pico
[LICENSE]: LICENSE
[RealDash]: http://realdash.net
["Upgraded" Metra 70-7552]: https://www.amazon.com/20-pin-Subaru-Headunit-Harness-Steering/dp/B01D9K3L44/uPWNsaWNrUmVkaXJlY3QmZG9Ob3RMb2dDbGljaz10cnVl
