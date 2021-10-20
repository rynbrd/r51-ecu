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

## Vehicle Implementation 

These sections are meant to give details on the R51's climate control
implementation. You do not need to know any of this information in order to use
the module.

## R51 Systems

The R51 contains two CAN connected ECUs critical to the operation of the
climate control system: the A/C Auto Amp the AV Control Unit.

The A/C Auto Amp is the computer which is connected to the climate control
hardware. It contains the relays and motor drivers and is connected to the rear
A/C controls. The A/C Auto Amp is connected to the various climate control
sensors and to the BCM to forward signals to the ECM which control activation
of the A/C compressor.

The AV Control Unit is the car's infotainment system. It consists of the AV
Control Unit itself, the Display Unit (LCD screen), and the A/C and AV Switch
Assembly. The AV Control Unit sends signals over CAN to control the state of
the climate control system.

### CAN Protocal

The A/C Auto Amp maintains the current state of the climate control system. It
sends CAN frames with the current state so that connected systems can display
that state to the user.

The AV Control Unit listens for CAN frame from the A/C Auto Amp and displays
the current state to the user. It also manages the state of most of the LEDs on
the A/C switches.

#### CAN Frame Definitions

The following CAN frames are exchanged by the climate control sytem.

| ID    | Purpose                                                                                  | Sender          | Listener        |
| ----- | ---------------------------------------------------------------------------------------- | --------------- | --------------- |
| 0x35D | External communication. Used to signal compressor state and control rear defrost heater. | AV Control Unit | BCM, ECU        |
| 0x540 | Button state. Requests changes to unit power, compressor, temperature, and mode.         | AV Control Unit | A/C Auto Amp    |
| 0x541 | Button state. Requests changes to fan speed and recirculation.                           | AV Control Unit | A/C Auto Amp    |
| 0x54A | Climate state. Sends current temperature configuration.                                  | A/C Auto Amp    | AV Control Unit |
| 0x54B | Climate state. Sends current power state, mode, fan speed, and recirculation.            | A/C Auto Amp    | AV Control Unit |
| 0x625 | Climate state. Sends current state of the rear defrost.                                  | Unknown         | Unknown         |

##### CAN Frame ID 35D
I have not fully decoded this this CAN frame but it appears to be sent form the
AV Control Unit or A/C Auto Amp to control the A/C compressor and rear defrost.

The eight bit controls the A/C compressor. 0 is off, 1 is on.

The 6th and 7th bits control the rear defrost. 00 is off, 11 is on.

Possible values:
* `00000000` defrost off, compressor off
* `00000001` defrost off, compressor on
* `00000110` defrost on, compressor off
* `00000111` defrost on, compressor on

The other bytes of the payload are repeated back. This requires the most
recently received frame to be stored in order to manipulate compressor and rear
defrost state.  

##### CAN Frame ID 540
This frame is sent by the AV Control Unit to initialize the A/C Auto Amp,
enable auto mode (this also turns on the unit), turn off the unit,
enable/disable the compressor, change the airflow direction (the "mode"), and
enable/disable dual zone control.

```
    +---------------------- 80 during init, 00 to ack, 60 when running
    |
    |  +------------------- 00 during init, 40 when running
    |  |
    |  |     +------------- driver temperature control
    |  |     |
    |  |     |  +---------- passenger temperature control
    |  |     |  |
    |  |     |  |  +------- temperature control and A/C compressor
    |  |     |  |  |
    |  |     |  |  |  +---- mode, front defrost, and dual zone
    |  |     |  |  |  |
540#60:40:00:00:00:00:04:00
    1  2  3  4  5  6  7  8
```

_Byte 1 and 2: Unit Status_

These bytes are changed in order to indicate the status of the unit: either
initializing, acknowledging first packages from the A/C Auto Amp, or normal
operation. Handshake is covered in a later section. Under normal operations
these bytes are 0x60 and 0x40.

_Byte 4: Driver Temperature Control_

This is an 8-bit unsigned integer indicating the temperature of the driver side
climate zone. It has an allowed value range from 0xF4 to 0x13. The value
increments or decrements by 1 when the driver side temperate knob is turned.
The value wraps around: Incrementing when the value is 0xFF moves to 0x00 as
one would expect of an unsigned integer.

These correspond to the temperature of the zone in Fahrenheit where 0xF4 is 60
degrees and 0x13 is 90.

It is likely possible to send a specific temperature. This is untested.

_Byte 5: Passenger Temperate Control_

This is a single byte indicating the temperate of the passenger side. It works
the same as the driver side value except the range is different. The range is
0x09 to 0x27.

_Byte 6: Temperature Control and A/C Compressor_

* Bits 1-2: Always 0.
* Bit 3: Flipped any time a temperature change occurs.
* Bit 4: Always 0.
* Bit 5: Indicates to the A/C Auto Amp whether or not to enable the A/C compressor. 1 is on, 0 is off as one might expect.
* Bits 6-8: Always 0.

Possible values:
* `00000000` temperature change; compressor off
* `00100000` temperature change; compressor off
* `00001000` temperature change; compressor on
* `00101000` temperature change; compressor on

_Byte 7: Mode, Front Defrost, and Dual Zone Control_

* Bit 1: Flipped to toggle the A/C compressor.
* Bit 2: Always 0.
* Bit 3: Flipped to enable the "auto" mode of the climate control unit.
* Bit 5: Flipped to toggle dual zone climate control.
* Bit 6: Always 1.
* Bit 7: Flipped to toggle front defrost.
* Bit 8: Flipped to toggle the "mode" - the direction of the airflow.

Examples:
* `00100100` auto mode toggled
* `00001100` dual zone climate control toggled
* `00000110` front defrost toggled
* `00000101` air flow mode toggled

##### CAN Frame ID 541
This frame is sent by the AV Control Unit to change fan speed and toggle
recirculation. 

```
    +---------------------- change fan speed
    |
    |  +------------------- toggle recirculation
    |  |
541#00:00:00:00:00:00:00:00
    1  2  3  4  5  6  7  8
```

_Byte 1: Change Fan Speed_

* Bits 1-2: Always 0.
* Bit 3: Flipped to increase fan speed.
* Bit 4: Flipped to decrease fan speed.

##### CAN Frame ID 54A
This frame is sent by the A/C Auto Amp to indicate the zone temperatures.

```
                +---------- current driver temperature
                |
                |  +------- current passenger temperature
                |  |
54A#3C:3E:7F:80:00:00:00:45
    1  2  3  4  5  6  7  8
```

_Byte 5: Driver Temperature_

This is an 8-bit unsigned integer representing the temperate of the driver side
climate zone in Fahrenheit. Values are from 60 to 90.

_Byte 6: Passenger Temperature_

This is an 8-bit unsigned integer representing the temperate of the passenger
side climate zone in Fahrenheit. Values are from 60 to 90.

##### Frame ID 54B
This frame is sent by the A/C Auto Amp to indicate unit status, mode, front
defrost, fan speed, recirculation, and dual climate zone mode.

```
    +---------------------- A/C compressor, auto mode, power off
    |
    |  +------------------- mode, defrost
    |  |
    |  |  +---------------- fan speed
    |  |  |
    |  |  |  +------------- recirculation, dual zone control
    |  |  |  |
54B#59:84:08:12:00:00:00:02
    1  2  3  4  5  6  7  8
```

_Byte 1: A/C Compressor, Auto Mode, Power Off_

The following values are possible:
* `0x41` A/C compressor Off, Auto Mode On
* `0x42` A/C compressor Off, Auto Mode Off
* `0x59` A/C compressor On, Auto Mode On
* `0x5A` A/C compressor On, Auto Mode Off
* `0xF2` Power Off

_Byte 2: Mode, Defrost_

The following values are possible:
* `0x00` Power Off
* `0x04` Face
* `0x08` Face and Feet
* `0x0C` Feet
* `0x10` Feet and Windshield
* `0x34` Windshield
* `0x84` Auto; Cooling; Face
* `0x88` Auto; Neutral; Face and Feet
* `0x8C` Auto; Heating; Feet

Windshield (defrost) is not possible in auto mode. Change the mode or enabling
defrost takes the unit out of auto mode. 

_Byte 3: Fan Speed_

This is an unsigned integer between 0x00 and 0x0F representing eight fan
speeds. The value increments and decrements by 0x02. The unit may sometimes
indicate half speeds. It is assumed that the value is rounded up in these
cases.

Fan speed it reported as 0x00 when the unit is off.

##### Frame ID 625
This frame is sent when the A/C compressor or rear defrost is toggled. The
sender and listeners are unknown at this point though it is assumed that it is
sent to indicate the current status of these items and not to change their
state.

```
    +---------------------- A/C compressor, rear defrost heater
    |
625#C0:03:00:00:00:00:00:00
    1  2  3  4  5  6  7  8
```

_Byte 1: A/C Compressor and Rear Defrost Heater_

* Bits 1-2: Always 1.
* Bits 3-5: Always 0.
* Bits 6-7: Indicates rear defrost heater status. 00 is off, 11 is on.
* Bit 8: Indicates A/C compressor status. 1 is on, 0 is off.

#### Initialization

The climate control systems boot when the ignition is turned on. At boot time
they both are in an "initialization" state. A handshake between the A/C Auto
Amp and AV Control Unit must occur before the climate control systems become
active.

![R51 Climate Handshake](images/r51_climate_handshake.png "R51 Climate Handshake")

During initalization the AV Control Unit repeats frames `540` and `541` at an
interval which varying between 100ms and 400ms. They each have the same payload:
```
540#8000000000000000
541#8000000000000000
```

When the A/C Auto Amp receives the above initialization packets it restores the
climate control to the last saved state. It sends frames `54B` and `54A` (in
that order) to indicate the running state:
```
54B#F200002400000000
54A#3C3E7F800000002C
```

The AV Control Unit reponds with updated `540` and `541` frames to acknowledge:
```
540#0000000000000000
541#0000000000000000
```

From this point forward the system is initialized and begins normal operations.

#### Normal Operation

Under normal operation the A/C Auto Amp and AV Control Unit send frames at a
regular interval to indicate their current state. A state change in a unit
causes them to send updated frames immediately.

The A/C Auto Amp responds to `540` and `541` frames from the AV Control Unit.

![R51 Climate Operation](images/r51_climate_operation.png "R51 Climate Operation")

The `540` and `541` frames appear to repeat at an interval which varies between
100ms and 1200ms.

# License
The contents of this repository are covered under the GPLv3. See the [LICENSE]
file for full details.


[CAN Bed v1]: https://www.seeedstudio.com/CANBed-Arduino-CAN-BUS-Development-Kit-Atmega32U4-with-MCP2515-and-MCP2551-p-4365.html
[LICENSE]: LICENSE
[RealDash]: http://realdash.net
