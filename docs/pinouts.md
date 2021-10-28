# R51 Pinouts

This page documents the pinouts for the ECU's related to this project.

## AV Control Unit Pinout

The R51 AV Control Unit (head unit) has 7 connectors. They're labeled in the
service manual as M42, M43, M44, M45, M46, M69, and M70. Pins on the connectors
are labeled 1 through 119. I've attempted to document their relevant
functionality here.

![R51 AV Control Unit Connectors](images/r51_av_connectors.jpg "R51 AV Control Unit Connectors")

The following housings or harness parts are assumed to be compatible. This is
pending confirmation.

| Connector | Pin | Housing      | Harness or Alternate Part   | 
| --------- | --- | ------------ | --------------------------- |
| M42       |  20 |              | ["Upgraded" Metra 70-7552]  |
| M43       |  12 |              |                             |
| M44       |   3 |              | Metra 40-NI12               |
| M45       |  24 | TE 1376103-1 |                             |
| M46       |  16 | TE 1376106-1 |                             |
| M69       |  12 | TE 1379675-5 | Metra 70-7554, TE 1379675-1 |
| M70       |  32 | TE 1473799-1 |                             |

The "upgraded" 70-7552 is not a Metra product but contains all the necessary
wires to connect to the stock harness. There are extras, particularly if your
system is equipped with the Bose sound system. I recommend using a pin removal
tool to remove the wires you do not need from this harness.

The Metra 70-7554 contains two pigtails: one for M69 for the Bose amplifier
pre-outs and one for M42. The M42 pigtail only has a few of its pins populated
and isn't suitable for use. You must either use the upgraded 70-7552 or
purchase additional terminals and crimp your own.

The pre-amp pigtail does not contain an amp-on wire. I recommend you use a pin
removal tool to remove one of the wires from the included M42 connector and
insert it for your amp-on wire.

### Connector M42

Contains wires for the speakers, accessor power, dash illumination, and
steering switch. The speaker terminals are not connected for R51s equiped with
the Bose amplifier.

| Pin | Color | Signal Name | Purpose                            |
| --- | ----- | ----------- | ---------------------------------- |
|   1 |       |             | not connected                      |
|   2 | BR    | FR_SP_LH+   | front left speaker (+)             |
|   3 | L     | FR_SP_LH-   | front left speaker (-)             |
|   4 | G     | RR_SP_LH+   | rear left speaker (+)              |
|   5 | B     | RR_SP_LH-   | rear left speaker (-)              |
|   6 | Y     | STRG_SW_A   | steering wheel switch up           |
|   7 | G/Y   | ACC         | accessory power, 10A fuse          |
|   8 |       |             | not connected                      |
|   9 | V     | ILL+        | dash illumination, battery voltage |
|  10 |       |             | not connected                      |
|  11 | LG    | FR_SP_RH+   | front right speaker (+)            |
|  12 | R     | FR_SP_RH-   | front right speaker (-)            |
|  13 | GR    | RR_SP_RH+   | rear right speaker (+)             |
|  14 | O     | RR_SP_RH-   | rear right speaker (-)             |
|  15 |       | STRG_SW_GND | steering wheel switch ground       |
|  16 | BR    | STRG_SW_B   | steering wheel switch down         |
|  17 |       |             | not connected                      |
|  18 |       |             | not connected                      |
|  19 | Y     | +B          | battery power, 20A fuse            |
|  20 | B     | GND         | ground                             |

The steering wheel pins connect to the spiral cable with a ground line.

Steering switch function approximate resistance:

| Function      | Line      | Resistance |
| ------------- | --------- | ---------- |
| Volume (up)   | STRG_SW_A |        487 |
| Seek (up)     | STRG_SW_A |        165 |
| Power         | STRG_SW_A |          0 |
| Volume (down) | STRG_SW_B |        487 |
| Seek (down)   | STRG_SW_B |        165 |
| Mode          | STRG_SW_B |          0 |

### Connector M43

Connected to satellite radio tuner if present.

| Pin | Color | Signal Name  | Purpose         |
| --- | ----- | ------------ | --------------- |
|  21 | G     | N_BUs_LH-    |                 |
|  22 | R     | N_BUS_LH+    |                 |
|  23 | W     | N_BUS_RH-    |                 |
|  24 | B     | N_BUS_RH+    |                 |
|  25 |       | N_BUS_SH     | shield          |
|  26 |       | DATA_GND     |                 |
|  27 |       |              | not connected   |
|  28 | O     | REQ_(TO HU)  | satellite tuner |
|  29 | P     | RX_(TO HU)   | satellite tuner |
|  30 | L     | TX_(FROM HU) | satellite tuner |
|  31 |       |              | not connected   |
|  32 |       |              | not connected   |

### Connector M44

This is the antenna connector.

| Pin | Color | Signal Name  | Purpose         |
| --- | ----- | ------------ | --------------- |
|  33 | B     |              | powered antenna |
|  34 | B     | ANT AMP      | antenna signal  |
|  35 | B     | ANT AMP      | antenna signal  |

### Connector M45

This connector is for video output.

| Pin | Color | Signal Name  | Purpose       |
| --- | ----- | ------------ | ------------- |
|  36 | G     | COMP_OUT+    |               |
|  37 | R     | COMP_OUT-    |               |
|  38 | R     | B            |               |
|  39 | B     | G            |               |
|  40 | W     | R            |               |
|  41 | R     | RGB_SYNC     |               |
|  42 |       | RGB_SYNC_GND |               |
|  43 | G     | YS           |               |
|  44 | LG    | DISP_IT      |               |
|  45 | B     | HP           |               |
|  46 | BR    | SIG_GND      |               |
|  47 | R     | SIG_VCC      |               |
|  48 |       |              | not connected |
|  49 |       | COMP_SHIELD  |               |
|  50 |       | RGB_GND      |               |
|  51 |       |              | not connected |
|  52 |       |              | not connected |
|  53 |       |              | not connected |
|  54 | B     | GND          |               |
|  55 |       | SHIELD       |               |
|  56 | V     | IT_DISP      |               |
|  57 | W     | VP           |               |
|  58 | SB    | INV_GND      |               |
|  59 | O     | INV_VCC      |               |
 
### Connector M46

This connector is for video input.

| Pin | Color | Signal Name    | Purpose                 |
| --- | ----- | -------------- | ----------------------- |
|  60 |       |                | not connected           |
|  61 |       |                | not connected           |
|  62 |       |                | not connected           |
|  63 |       |                | not connected           |
|  64 | W     | VTR+           | backup camera           |
|  65 | B     | VTR-           | backup camera           |
|  66 | G     | COMP_IN+       | DVD player              |
|  67 |       |                | not connected           |
|  68 | BR    | RV_CAM_SIG     | backup camera signal    |
|  69 |       |                | not connected           |
|  70 |       |                | not connected           |
|  71 |       |                | not connected           |
|  72 |       | COMP_IN_SHIELD |                         |
|  73 |       | GND            | shield                  |
|  74 | R     | COMP_IN-       | DVD player              |
|  75 |       |                | not connected           |

### Connector M69

This connects to the amplifier. It contains the pre-outs and amplifier on line.

| Pin | Color | Signal Name    | Purpose                       |
| --- | ----- | -------------- | ----------------------------- |
| 108 | R     | RR_RH_PRE+     | rear right pre-out (+)        |
| 109 | G     | FR_RH_PRE+     | front right pre-out (+)       |
| 110 | B     | AMP_ON         | amplifier on, battery voltage |
| 111 |       |                | shield                        |
| 112 | L     | RR_LH_PRE+     | rear left pre-out (+)         |
| 113 | R     | FR_LH_PRE+     | front left pre-out (+)        |
| 114 | B     | RR_RH_PRE-     | rear right pre-out (-)        |
| 115 | W     | FR_RH_PRE-     | front right pre-out (-)       |
| 116 |       |                | not connected                 |
| 117 |       |                | not connected                 |
| 118 | Y     | RR_LH_PRE-     | rear left pre-out (-)         |
| 119 | B     | FR_LH_PRE-     | front left pre-out (-)        |

### Connector M70

This connects to the various CAN busses, aux input, and reverse/park/speed
inputs.

| Pin | Color | Signal Name       | Purpose                      |
| --- | ----- | ----------------- | ---------------------------- |
|  76 | R     |  HP_RH-           |                              |
|  77 | B     |  HP_RH+           |                              |
|  78 |       |                   | not connected                |
|  79 |       |                   | not connected                |
|  80 |       |                   | not connected                |
|  81 |       |                   | not connected                |
|  82 | G     |  AUDIO_BUS_RH-    |                              |
|  83 | R     |  AUDIO_BUS_RH+    |                              |
|  84 |       |                   | not connected                |
|  85 | B     |  GND              | ground                       |
|  86 | L     |  CAN_H            | vehicle CAN high             |
|  87 | P     |  CAN_L            | vehicle CAN low              |
|  88 | L     |  M_CAN1_H         | switch assembly CAN high     |
|  89 | P     |  M_CAN1_L         | switch assembly CAN low      |
|  90 | L     |  M_CAN2_H         | DVD player CAN high          |
|  91 | P     |  M_CAN2_L         | DVD player CAN low           |
|  92 | W     |  HP_LH-           |                              |
|  93 | G     |  HP_LH+           |                              |
|  94 |       |  HP_SHIELD        |                              |
|  95 | G     |  AUX_AUDIO_RH+    | aux input right (+)          |
|  96 | L     |  AUX_AUDIO_LH+    | aux input left (+)           |
|  97 | Y     |  AUX_GND          | aux input ground             |
|  98 | B     |  AUDIO_BUS_LH-    |                              |
|  99 | W     |  AUDIO_BUS_LH+    |                              |
| 100 |       |  AUDIO_BUS_SHIELD |                              |
| 101 | GR    |  SW_GND           | switch assembly ground       |
| 102 |       |                   | not connected                |
| 103 | SB    |  CD_EJECT         | CD eject button              |
| 104 | W/G   |  IGN              | ignition power, 10A fuse     |
| 105 | W     |  REVERSE_SIG      | reverse signal (+)           |
| 106 | G     |  PKB_SIG          | parking break enable signal  |
| 107 | LG    |  SPEED_8P         | 8-pulse vehicle speed signal |

