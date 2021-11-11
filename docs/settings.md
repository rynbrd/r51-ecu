# R51 BCM Settings Control

The R51's AV Control Unit manages BCM settings in addition to climate control
and audio. These settings are managed via the "Comfort & Conv." settings menu
in the AV Control Unit.

## Settings

The following settings are available:

| Setting                        | Available Values                         |
| ------------------------------ | ---------------------------------------- |
| Auto Interior Illumination     | on, off
| Auto Headlights Sensitivity    | 1, 2, 3, 4                               |
| Auto Headlights Off Delay      | 0s, 30s, 45s, 60s, 90s, 120s, 150s, 180s |
| Speed Sensing Wiper Interval   | on, off                                  |
| Remote Key Response Horn       | on, off                                  |
| Remote Key Response Lights     | on, off, unlock, lock                    |
| Auto Re-Lock Time              | off, 1m, 5m                              |
| Selective Door Unlock          | on, off                                  |
| Slide Driver Seat Back on Exit | on, off                                  |
| Return All Settings to Default | button click, no values available        |


## CAN Protocol

There are two sets of command/response frames: 0x71E/0x72E and 0x71F/0x72F.
These operate identically but cover different commands.

The BCM appears to have a settings mode which is activated and deactivated by
specific command frames:
```
# enter settings mode
>> 71E#02:10:C0:FF:FF:FF:FF:FF
<< 72E#02:50:C0:FF:FF:FF:FF:FF

# send settings command frames here

# exit settings mode
>> 71E#02:10:81:FF:FF:FF:FF:FF
<< 72E#02:50:81:FF:FF:FF:FF:FF
```

Once settings mode is activated the AV Control Unit may changes settings and/or
request the values of the current settings.

Change a settings value:
```
>> 71E#03:3B:10:00:FF:FF:FF:FF
<< 72E#02:7B:10:FF:FF:FF:FF:FF
```

Request some settings values:
```
>> 71E#02:21:01:FF:FF:FF:FF:FF
<< 72E#10:11:61:01:00:1E:24:00
```

The AV Control Unit enters settings mode and requests all settings values when
the user enters the settings screen.

The AV Control Unit also appears to perform some requests during boot. It is
unclear what these commands are for.

### Changing a Setting

The AV Control Unit puts the BCM into settings mode in order to change a single
setting. Once in settings mode a command frame is sent to change the setting
and an acknowledgement is received. Then the updated state is requested from
the BCM and displayed.

The following example sequence turns off the Auto Interior Illumination:
```
# Enter settings mode.
>> 71E#02:10:C0:FF:FF:FF:FF:FF
<< 72E#02:50:C0:FF:FF:FF:FF:FF

# Perform the settings update. Byte 2 identifies the setting. Byte 3 contains
the value to set it to. The BCM acknowledges the command by responding with the
same setting identifier (0x10).
>> 71E#03:3B:10:01:FF:FF:FF:FF
<< 72E#02:7B:10:FF:FF:FF:FF:FF

# Request the current state. There are two state request commands. This is the
first.
>> 71E#02:21:01:FF:FF:FF:FF:FF
<< 72E#10:11:61:01:20:1E:24:00

# The second state request command. The BCM responds to this command with two
state frames.
>> 71E#30:00:0A:FF:FF:FF:FF:FF
<< 72E#21:E0:00:0A:40:01:64:00
<< 72E#22:94:00:00:47:FF:FF:FF

# Exit settings mode.
>> 71E#02:10:81:FF:FF:FF:FF:FF
<< 72E#02:50:81:FF:FF:FF:FF:FF
```

#### Auto Interior Illumination
Command Frame:      `0x71E`
Command Identifier: `0x10`
State Identifier:   `0x10`
State Byte:         `4`
State Bit Start:    `5`
State Bit Length:   `1`

| State Value | Description |
| ----------- | ----------- |
| 0           | off         |
| 1           | on          |

Examples:
```
>> 71E#03:3B:10:01:FF:FF:FF:FF
<< 72E#02:7B:10:FF:FF:FF:FF:FF
```

```
>> 71E#02:21:01:FF:FF:FF:FF:FF
<< 72E#10:11:61:01:20:1E:24:00
```

#### Auto Headlights Sensitivity
Command Frame:      `0x71E`
Command Identifier: `0x37`
State Identifier:   `0x21`
State Byte:         `2`
State Bit Start:    `2`
State Bit Length:   `2`

| State Value | Description |
| ----------- | ----------- |
| 11          | 1           |
| 00          | 2           |
| 01          | 3           |
| 10          | 4           |

Examples:
```
>> 71E#03:3B:37:03:FF:FF:FF:FF
<< 72E#02:7B:37:FF:FF:FF:FF:FF
```

```
>> 71E#30:00:0A:FF:FF:FF:FF:FF
<< 72E#21:E0:04:0A:40:01:64:00
```

#### Auto Headlights Off Delay
Command Frame:      `0x71E`
Command Identifier: `0x39`
State Identifier:   `0x21`
State Byte:         `2+3`
State Bit Start:    `2[0]+3[6:7]`
State Bit Length:   `2`

| State Value | Description |
| ----------- | ----------- |
| 0x00 (0000) | 45s         |
| 0x01 (0001) | 0s          |
| 0x02 (0010) | 30s         |
| 0x03 (0011) | 60s         |
| 0x04 (0100) | 90s         |
| 0x05 (0101) | 120s        |
| 0x06 (0110) | 150s        |
| 0x07 (0111) | 180s        |

Examples:
```
>> 71E#03:3B:37:03:FF:FF:FF:FF
<< 72E#02:7B:37:FF:FF:FF:FF:FF
```

```
>> 71E#30:00:0A:FF:FF:FF:FF:FF
<< 72E#21:E0:0C:0A:40:01:64:00
```

#### Speed Sensing Wiper Interval
Command Frame:      `0x71E`
Command Identifier: `0x47`
State Identifier:   `0x22`
State Byte:         `1`
State Bit Start:    `7`
State Bit Length:   `1`

| State Value | Description |
| ----------- | ----------- |
| 1           | off         |
| 0           | on          |

Examples:
```
>> 71E#03:3B:47:00:FF:FF:FF:FF
<< 72E#02:7B:47:FF:FF:FF:FF:FF
```

```
>> 71E#30:00:0A:FF:FF:FF:FF:FF
<< 72E#22:14:00:00:47:FF:FF:FF
```

#### Remote Key Response Horn
Command Frame:      `0x71E`
Command Identifier: `0x2A`
State Identifier:   `0x10`
State Byte:         `7`
State Bit Start:    `3`
State Bit Length:   `1`

| State Value | Description |
| ----------- | ----------- |
| 0           | off         |
| 1           | on          |

Examples:
```
>> 71E#03:3B:2A:01:FF:FF:FF:FF
<< 72E#02:7B:2A:FF:FF:FF:FF:FF
```

```
>> 71E#02:21:01:FF:FF:FF:FF:FF
<< 72E#10:11:61:01:00:1E:24:08
```

#### Remote Key Response Lights
Command Frame:      `0x71E`
Command Identifier: `0x2E`
State Identifier:   `0x21`
State Byte:         `1`
State Bit Start:    `6`
State Bit Length:   `2`

| State Value | Description |
| ----------- | ----------- |
| 00          | off         |
| 01          | unlock      |
| 10          | lock        |
| 11          | on          |

Examples:
```
>> 71E#03:3B:2E:01:FF:FF:FF:FF
<< 72E#02:7B:2E:FF:FF:FF:FF:FF
```

```
>> 71E#30:00:0A:FF:FF:FF:FF:FF
<< 72E#21:60:04:8A:40:01:64:00
```

#### Auto Re-Lock Time
Command Frame:      `0x71E`
Command Identifier: `0x2F`
State Identifier:   `0x21`
State Byte:         `1`
State Bit Start:    `4`
State Bit Length:   `2`

| State Value | Description |
| ----------- | ----------- |
| 00          | 1m          |
| 01          | off         |
| 10          | 5m          |

Examples:
```
>> 71E#03:3B:2F:00:FF:FF:FF:FF
<< 72E#02:7B:2F:FF:FF:FF:FF:FF
```

```
>> 71E#30:00:0A:FF:FF:FF:FF:FF
<< 72E#21:C0:04:8A:40:01:64:00
```

#### Selective Door Unlock
Command Frame:      `0x71E`
Command Identifier: `0x02`
State Identifier:   `0x10`
State Byte:         `4`
State Bit Start:    `7`
State Bit Length:   `1`

| State Value | Description |
| ----------- | ----------- |
| 0           | off         |
| 1           | on          |

Examples:
```
>> 71E#03:3B:02:01:FF:FF:FF:FF
<< 72E#02:7B:02:FF:FF:FF:FF:FF
```

```
>> 71E#02:21:01:FF:FF:FF:FF:FF
<< 72E#10:11:61:01:80:1E:24:00
```

#### Slide Driver Seat Back on Exit
Command Frame:      `0x71F`
Command Identifier: `0x01`
State Identifier:   `0x05`
State Byte:         `3`
State Bit Start:    `0`
State Bit Length:   `1`

| State Value | Description |
| ----------- | ----------- |
| 0           | off         |
| 1           | on          |

Examples:
```
>> 71F#03:3B:01:00:FF:FF:FF:FF
<< 72F#02:7B:01:FF:FF:FF:FF:FF
```

```
>> 71F#02:21:01:FF:FF:FF:FF:FF
<< 72F#05:61:01:00:00:00:FF:FF
```

### Return All Settings to Default
```
# Enter settings mode for first command set.
<< 71E#02:10:C0:FF:FF:FF:FF:FF
<< 72E#02:50:C0:FF:FF:FF:FF:FF

# Send reset all command.
<< 71E#03:3B:1F:00:FF:FF:FF:FF
<< 72E#02:7B:1F:FF:FF:FF:FF:FF

# Request current state, part 1.
<< 71E#02:21:01:FF:FF:FF:FF:FF
<< 72E#10:11:61:01:80:1E:24:00

# Request current state, part 2.
<< 71E#30:00:0A:FF:FF:FF:FF:FF
<< 72E#21:C0:00:0A:40:01:64:00
<< 72E#22:94:00:00:47:FF:FF:FF

# Exit settings mode.
<< 71E#02:10:81:FF:FF:FF:FF:FF
<< 72E#02:50:81:FF:FF:FF:FF:FF

# Enter settings mode for second command set.
<< 71F#02:10:C0:FF:FF:FF:FF:FF
<< 72F#02:50:C0:FF:FF:FF:FF:FF

# Send reset all command.
<< 71F#03:3B:1F:00:FF:FF:FF:FF
<< 72F#02:7B:1F:FF:FF:FF:FF:FF

# Request current state.
<< 71F#02:21:01:FF:FF:FF:FF:FF
<< 72F#05:61:01:01:00:00:FF:FF

# Exit settings mode.
<< 71F#02:10:81:FF:FF:FF:FF:FF
<< 72F#02:50:81:FF:FF:FF:FF:FF
```

#### CAN Frame ID 0x72E

This is the response frame for 0x71E request frames. It ACK's specific request
and is used to provide the caller with settings state.
```
72E#02:50:C0:FF:FF:FF:FF:FF
    0  1  2  3  4  5  6  7
```

_Byte 0_

Observed values:
* `0x02` Frame contains command response.
* `0x10` Frame contains state A.
* `0x21` Frame contains state B.
* `0x22` Frame contains state C.

_Byte 1_

For `0x21` state:
Bits 6-7: Remote Key Response Lights
* `0x20 (00100000)` Off
* `0x60 (01100000)` Unlock
* `0xA0 (10100000)` Lock
* `0xE0 (11100000)` On

Bits 4-5: Auto Re-Lock Time
* `0xC0 (11000000)` 1m
* `0xD0 (11010000)` off
* `0xE0 (11100000)` 5m

For `0x22` state:
Bit 7: Speed Sensing Wiper Interval 
* `0x14 (00010100)` on
* `0x94 (10010100)` off

_Byte 2_

For `0x21` state:
Bit 0: Auto Headlights Off Delay
* 0: Select values 0s, 30s, 45s, 60s
* 1: Select values 90s, 120s, 150s, 180s

Bits 2-3: Auto Headlights Sensitivity
* `0x0C (00001100)` 1
* `0x00 (00000000)` 2
* `0x04 (00000100)` 3
* `0x08 (00001000)` 4

_Byte 3_

For `0x21` state:
Bits 6-7: Auto Headlights Off Delay
* `0x0A (00001010)` 45s, 90s
* `0x4A (01001010)` 0s, 120s
* `0x8A (10001010)` 30s, 150s
* `0xCA (11001010)` 60s, 180s

Combining the bits from byte 2 and 3 for the Auto Deadlights Off Delay creates
the following 4 bit numbers:
* `0x00 (0000)` 45s
* `0x01 (0001)` 0s
* `0x02 (0010)` 30s
* `0x03 (0011)` 60s
* `0x04 (0100)` 90s
* `0x05 (0101)` 120s
* `0x06 (0110)` 150s
* `0x07 (0111)` 180s

_Byte 4_

For `0x10` state:
* Bit 5: Auto Interior Illumination; 1 is on, 0 is off.
* Bit 7: Selective Door Unlock; 1 is on, 0 is off.

_Byte 5_

_Byte 6_

_Byte 7_

For `0x10` state:
* `0x00` Remote Key Response Horn set to On.
* `0x08` Remote Key Response Horn set to Off.

#### CAN Frame ID 0x72F

This is the response frame for 0x71F request frames. It ACK's specific request
and is used to provide the caller with settings state.
```
72E#02:50:C0:FF:FF:FF:FF:FF
    0  1  2  3  4  5  6  7
```

_Byte 0_

Observed values:
* `0x02` Frame contains command response.
* `0x05` Frame contains state A.

_Byte 1_

_Byte 2_

_Byte 3_

For `0x05` state:
* Bit 0: Slide Driver Seat Back on Exit; 1 is On, 0 is Off.

_Byte 4_

_Byte 5_

_Byte 6_

_Byte 7_

