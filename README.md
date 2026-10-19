# Intellicart

Intellicart is an open hardware flash cartridge for the Mattel Intellivision.
It runs on a Teensy 4.1, loads Intellivision cartridge images from a microSD
card, and uses an OLED display plus three cartridge buttons for selection.

## Project status

### Firmware and user interface: current baseline

The current Teensy 4.1 firmware builds with Arduino CLI, uploads successfully,
and has been exercised through the OLED/button interface and microSD browser.
It includes the Intellicart branding, startup help, title-and-publisher browser
view, bidirectional browser wraparound, button-release debouncing, and return
to the last selected game after leaving gameplay.

This is the baseline for future firmware development. Changes to game support,
the user interface, or the bus emulator should be developed and tested against
this firmware independently of physical cartridge assembly.

### Physical cartridge bring-up: pending live test

The remaining hardware milestone is to fit the 74x245 bus-transceiver ICs and
perform a live test on an Intellivision console. That test should validate:

- cartridge insertion, power-up, and reset behavior;
- OLED/button operation while installed in the console;
- selection and loading of a known-good standard ROM from microSD;
- controller operation, game reset, and return to the browser;
- reliable bus behavior before testing mapper-specific titles.

Treat any issues found during the live test as hardware-integration findings,
not regressions in the verified firmware baseline, unless the evidence points
to the firmware.

## Firmware

The Arduino sketch is
[`development/teensy/intellicart/intellicart.ino`](development/teensy/intellicart/intellicart.ino).

### Requirements

- Arduino CLI or Arduino IDE with the **Teensy 4.1** board package installed.
- The PJRC Teensy Boards Manager URL:
  `https://www.pjrc.com/teensy/package_teensy_index.json`
- Arduino libraries:
  - `Adafruit GFX Library`
  - `Adafruit SSD1306`

Do **not** install a generic `SdFat` library through Arduino Library Manager.
Intellicart uses Teensy's bundled, SDIO-compatible SdFat implementation via
`SD.h`.

### Build and upload with Arduino CLI

```bash
arduino-cli config add board_manager.additional_urls \
  https://www.pjrc.com/teensy/package_teensy_index.json
arduino-cli core update-index
arduino-cli core install teensy:avr
arduino-cli lib install "Adafruit GFX Library" "Adafruit SSD1306"

arduino-cli compile --upload \
  --fqbn teensy:avr:teensy41 \
  --port <teensy-port> \
  development/teensy/intellicart
```

Find the current Teensy port with:

```bash
arduino-cli board list
```

On Linux, install PJRC's udev rule before uploading as a non-root user. See
the [Teensy 4.1 documentation](https://www.pjrc.com/store/teensy41.html).

## Preparing the microSD card

Use a 1--32 GB microSD card formatted as FAT or FAT32. Put the fallback
configuration below in the card root as `0.cfg`:

```ini
[mapping]
$0000 - $1FFF = $5000 ; 8K to $5000 - $6FFF
$2000 - $2FFF = $D000 ; 4K to $D000 - $DFFF
$3000 - $3FFF = $F000 ; 4K to $F000 - $FFFF
```

Copy legally obtained Intellivision game dumps as `.bin` files. A game can
have a same-named `.cfg` file for its specific memory mapping:

```text
SD_CARD/
├── 0.cfg
├── Pitfall.bin
├── Pitfall.cfg
├── Astrosmash.bin
└── Astrosmash.cfg
```

`0.cfg` is used when no same-named CFG is present. It supports many standard
ROMs, but bank-switched or RAM-mapped titles may require their own CFG file.
The browser only displays `.bin` files; CFG files remain hidden.

Files may be stored in the root or a root-level folder. Keep names below 45
characters and avoid unusual punctuation. `.int` images use the same binary
format and may be renamed to `.bin`; `.rom` images must first be converted to
BIN plus CFG.

## Current SD-card catalogue

[`catalog/SD-CARD-CATALOG.md`](catalog/SD-CARD-CATALOG.md) records every
numbered image on the prepared `INTELLICART` card, including its source title,
year, publisher, category, byte size, and exact card filename. It is intended
as the reference for later compatibility and mapper testing. The catalogue
also supports local cover thumbnails in `catalog/covers/`, using a three-digit
catalogue number such as `053.png`.

## Controls

At startup, pressing any cartridge button opens the browser. The OLED shows:

```text
          Intellicart
PB1/PB3: Navigate
PB2: Start
PB3: Menu
```

| Context | PB1 | PB2 | PB3 |
| --- | --- | --- | --- |
| Browser | Next item | Open folder / start game | Previous item |
| Game running | Restart current game | — | Return to browser |

Browser navigation wraps in both directions. Returning to the browser from a
game restores the previously selected game and directory. The browser displays
a game title and publisher when the filename encodes them; otherwise it shows
the title and file size. Long display text is shortened with an ellipsis.

The small centre button on the Teensy 4.1 is a **Program** button, not a game
reset button. It enters Teensy's bootloader for firmware recovery/uploading.
Do not hold it for 13--17 seconds: that restores the default Teensy blink
program and erases the Intellicart firmware.

## Repository layout

| Path | Contents |
| --- | --- |
| [`board/project/`](board/project/) | KiCad schematic, PCB, project configuration, routing, and BOM files. |
| [`board/gerbers/`](board/gerbers/) | Generated fabrication deliverables. |
| [`development/teensy/intellicart/`](development/teensy/intellicart/) | Teensy 4.1 firmware and startup ROM image. |
| [`development/checkers-disassembly/`](development/checkers-disassembly/) | CP1610 disassembly and analysis workspace for the Checkers ROM. |
| [`catalog/`](catalog/) | Current SD-card inventory, optional cover artwork, and test reference. |
| [`scripts/generate_sd_catalog.py`](scripts/generate_sd_catalog.py) | Rebuilds the SD-card catalogue from the card and source ROM set. |
| [`manual/`](manual/) | User manual source, assets, and compiled PDF. |
| [`rom/`](rom/) | ROM images used for development and research. |

## Hardware and manual

The KiCad project is
[`board/project/intellicart.kicad_pro`](board/project/intellicart.kicad_pro).
For manufacturing, use the complete matching output set in
[`board/gerbers/`](board/gerbers/).

The editable user-manual source is [`manual/UserManual.tex`](manual/UserManual.tex).
To rebuild its PDF:

```bash
cd manual
latexmk -pdf UserManual.tex
```

## Licensing and ROMs

The hardware and project files are provided under the repository's
[license](LICENSE). ROM images may be subject to their own rights; use and
distribute them only where you have the appropriate authorization.
