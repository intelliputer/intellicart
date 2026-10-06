## Intellicart

Intellicart is a flash-cartridge project for the Mattel Intellivision. It loads Intellivision program images from a micro SD card and presents a small OLED/button interface for selecting and starting them on original hardware.

This repository brings together the hardware design, fabrication package, firmware, user documentation, and a small reverse-engineering workspace for experimenting with human--agent gameplay.

## Repository layout

| Path | Contents |
| --- | --- |
| [`board/project/`](board/project/) | KiCad schematic, PCB, project configuration, routing, and BOM files. |
| [`board/gerbers/`](board/gerbers/) | Generated fabrication deliverables: Gerbers, drill files, drill maps, and the Gerber job file. |
| [`development/arduino/`](development/arduino/) | Cartridge-controller firmware source. |
| [`development/checkers-disassembly/`](development/checkers-disassembly/) | CP1610 disassembly and analysis workspace for the bundled Checkers ROM. |
| [`manual/`](manual/) | Intellicart user manual as LaTeX source, diagram asset, and compiled PDF. |
| [`rom/`](rom/) | ROM images used for development and research. |
| [`images/`](images/) | Project imagery. |

## Hardware

The KiCad project is [`board/project/intellicart.kicad_pro`](board/project/intellicart.kicad_pro). For manufacturing, use the complete, matching output set in [`board/gerbers/`](board/gerbers/). KiCad is configured to place newly plotted outputs there (`../gerbers/` relative to the project directory).

## User manual

The editable source is [`manual/UserManual.tex`](manual/UserManual.tex). To rebuild the PDF:

```bash
cd manual
latexmk -pdf UserManual.tex
```

## Checkers human--agent gameplay research

[`rom/Checkers (1979) (Mattel).int`](rom/Checkers%20(1979)%20(Mattel).int) is an 8 KiB CP1610 ROM mapped at `$5000`--`$5FFF`. Its generated assembly listing lives in [`development/checkers-disassembly/checkers.asm`](development/checkers-disassembly/checkers.asm).

To regenerate that listing, place the SDK1600 repository adjacent to this one and run:

```bash
development/checkers-disassembly/build-disassembly.sh
```

The script builds a temporary native `dasm1600` executable from `../sdk1600/src/dasm/dasm1600.c`, so it does not add a compiled tool to this repository. The original ROM remains unmodified.

## Licensing and ROMs

The hardware and project files are provided under the repository's [license](LICENSE). ROM images may be subject to their own rights; use and distribute them only where you have the appropriate authorization.
