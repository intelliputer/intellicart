# J1 cartridge-interface review

Status: **review only — no electrical change has been made.**

This compares the current PCB/schematic netlist with the canonical
Intellivision cartridge-port table maintained alongside the Intelliputer
Connector project. `NC` means the current design intentionally leaves that
finger electrically isolated; it does not mean the console contact is unused.

| J1 | Canonical port function | Current net/treatment | Review result |
| ---: | --- | --- | --- |
| 1 | GND | GND | matches |
| 2 | GND on Intellivision I; System Changer video input on II | NC | deliberately isolated; retain unless compatibility design says otherwise |
| 3 | `~MSYNC` | `~msync'` | matches |
| 4 | `CBLNK` | NC | deliberately isolated |
| 5 | DB7 | `d7'` | matches |
| 6 | external audio input | NC | deliberately isolated |
| 7 | DB8 | `d8'` | matches |
| 8 | external video input on Intellivision I | **GND** | **blocker: must be reviewed** |
| 9 | DB6 | `d6'` | matches |
| 10 | 3.579545 MHz master-clock output | **GND** | **blocker: must be reviewed** |
| 11 | DB9 | `d9'` | matches |
| 12 | reset input | `rst'` | matches |
| 13 | DB5 | `d5'` | matches |
| 14 | `SR1` | NC | deliberately isolated |
| 15 | DB10 | `d10'` | matches |
| 16 | Intellivoice serial output | NC | deliberately isolated |
| 17 | DB4 | `d4'` | matches |
| 18 | Intellivoice serial input | NC | deliberately isolated |
| 19 | DB11 | `d11'` | matches |
| 20 | Intellivoice ROM-disable input | **GND** | **blocker: must be reviewed** |
| 21 | DB3 | `d3'` | matches |
| 22 | GND | GND | matches |
| 23 | DB12 | `d12'` | matches |
| 24 | GND | GND | matches |
| 25 | DB13 | `d13'` | matches |
| 26 | GND | GND | matches |
| 27 | DB2 | `d2'` | matches |
| 28 | GND | GND | matches |
| 29 | DB14 | `d14'` | matches |
| 30 | `~BUSAK` | NC | deliberately isolated |
| 31 | DB1 | `d1'` | matches |
| 32 | reflected BC1 input | looped to J1 42 | matches conventional loopback |
| 33 | DB0 | `d0'` | matches |
| 34 | reflected BC2 input | looped to J1 40 | matches conventional loopback |
| 35 | DB15 | `d15'` | matches |
| 36 | reflected BDIR input | looped to J1 38 | matches conventional loopback |
| 37 | raw BDIR output | `bdir'` | matches |
| 38 | duplicate raw BDIR output | looped to J1 36 | matches conventional loopback |
| 39 | raw BC2 output | `bc2'` | matches |
| 40 | duplicate raw BC2 output | looped to J1 34 | matches conventional loopback |
| 41 | raw BC1 output | `bc1'` | matches |
| 42 | duplicate raw BC1 output | looped to J1 32 | matches conventional loopback |
| 43 | +5 V | +5 V | matches |
| 44 | GND | GND | matches |

## Required decision before an electrical revision

The current design hard-grounds contacts 8, 10, and 20. Confirm whether this
was intentional for a documented hardware reason. If not, make each treatment
explicit in the schematic—normally isolated, routed, or deliberately
terminated—then update the PCB from that schematic. Do not edit PCB pad nets
or tracks directly.

After that decision, rerun DRC with schematic parity enabled and conduct the
first powered test with current limiting. Test an Intellivision and an
Intellivoice socket separately; the exposed alternate functions make those
not interchangeable tests.
