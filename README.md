# Nissan Pathfinder R51 Head Unit Adapter

This repo contains documentation and code to turn an Arduino into a CAN Bus
climate control adapter for the Nissan Pathfinder R51 (2005 - 2012). This is
compatible with Pathfinders that came stock with the screen based infotainment
systems. This adapter allows the replacement of the infotainment system and
will not work with the infotainment system installed.

This system may be compatible with with other Nissan models (Armada, QX56, etc)
of the same years.


## Arduino Hardware

The software targets the Arduino Lenoardo compatible [CAN Bed v1] but should
work with any MCP2515 implementation with some small updates.


## Software Interface

The software exposes a serial protocal compatible with [RealDash].

_Serial protocol TBD._


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
