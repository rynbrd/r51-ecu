# Nissan Pathfinder R51 Systems

This repo contains documentation and code to allow integration of custom ECU's
into the Nissan Pathfinder R51 (2005 - 2012). Some of these integrations may
work with other Nissan vehicles (Armada, QX56, etc) but have not been tested.

## Head Unit and Climate Control

The [climate](climate) directory contains an Arduino sketch that allows the
stock LCD screen infotainment system to be replaced by an aftermarket head
unit. It replaces the climate control systems with [RealDash]. The stock
infotainment system must be removed for this to operate properly.

The software targets the Arduino Lenoardo compatible [CAN Bed v1] but should
work with any MCP2515 implementation with some small updates.

The software exposes a serial protocal compatible with [RealDash]. Serial
protocol TBD.


## Vehicle Details 

* [Climate Control](docs/climate.md) - Operational details including CAN frame definitions.
* [Pinouts](docs/pinouts.md) - Pin outs and connector part numbers for the infotainment unit.


# License

The contents of this repository are covered under the GPLv3. See the [LICENSE]
file for full details.


[CAN Bed v1]: https://www.seeedstudio.com/CANBed-Arduino-CAN-BUS-Development-Kit-Atmega32U4-with-MCP2515-and-MCP2551-p-4365.html
[LICENSE]: LICENSE
[RealDash]: http://realdash.net
["Upgraded" Metra 70-7552]: https://www.amazon.com/20-pin-Subaru-Headunit-Harness-Steering/dp/B01D9K3L44/uPWNsaWNrUmVkaXJlY3QmZG9Ob3RMb2dDbGljaz10cnVl
