#!/usr/bin/env python3
"""Generate the Intellicart SD-card catalogue from a card and source ROM set."""

from __future__ import annotations

import argparse
import re
from pathlib import Path


YEAR_PATTERN = re.compile(r"\((?P<year>\d{4}(?:-\d{2})?)\)")
PUBLISHER_PATTERN = re.compile(r"^\s*\((?P<publisher>[^)]+)\)")
BYLINE_PATTERN = re.compile(r"^(?P<title>.+?)\s+by\s+(?P<publisher>.+)$", re.IGNORECASE)
CARD_PATTERN = re.compile(r"_(?P<index>\d{3})\.bin$", re.IGNORECASE)


def classify(title: str, year: str, flags: str) -> str:
    title_lower = title.lower()
    flags_lower = flags.lower()
    if (
        "bios" in title_lower
        or "executive rom" in title_lower
        or "exec-basic" in title_lower
        or title_lower.startswith("grom")
    ):
        return "System / BIOS"
    if re.search(r"\b(test|diagnostic)\b", title_lower):
        return "Diagnostic"
    if re.search(r"\bdemo\b", title_lower):
        return "Demo"
    if "prototype" in title_lower or "[p" in flags_lower:
        return "Prototype"
    if "hack" in title_lower:
        return "Hack"
    if "[a" in flags_lower:
        return "Alternate dump"
    if year != "Unknown" and int(year[:4]) >= 1999:
        return "Homebrew"
    return "Retail"


def parse_source_name(source: Path) -> tuple[str, str, str, str]:
    stem = source.stem
    match = YEAR_PATTERN.search(stem)
    if not match:
        return stem, "Unknown", "Unknown", "Retail"
    title = stem[: match.start()].rstrip()
    year = match.group("year")
    remainder = stem[match.end() :]
    publisher_match = PUBLISHER_PATTERN.match(remainder)
    if publisher_match:
        publisher = publisher_match.group("publisher")
        flags = remainder[publisher_match.end() :].strip()
    else:
        publisher = "Unknown"
        flags = remainder.strip()
        byline_match = BYLINE_PATTERN.match(title)
        if byline_match:
            title = byline_match.group("title")
            publisher = byline_match.group("publisher")
    return title, year, publisher, classify(title, year, flags)


def escape_table(value: str) -> str:
    return value.replace("|", "\\|")


def cover_cell(covers_dir: Path, number: int, title: str) -> str:
    for suffix in (".png", ".jpg", ".jpeg", ".webp"):
        cover = covers_dir / f"{number:03}{suffix}"
        if cover.is_file():
            return (
                f'<img src="covers/{cover.name}" alt="{title} cover" '
                'width="80">'
            )
    return "—"


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--card-dir", type=Path, required=True)
    parser.add_argument("--rom-dir", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument(
        "--covers-dir",
        type=Path,
        help="optional local cover-art directory (defaults to <output>/covers)",
    )
    args = parser.parse_args()
    covers_dir = args.covers_dir or args.output.parent / "covers"

    card_files = sorted(
        (path for path in args.card_dir.glob("*.bin") if CARD_PATTERN.search(path.name)),
        key=lambda path: int(CARD_PATTERN.search(path.name).group("index")),
    )
    # The card was populated with `LC_ALL=C sort`, which sorts UTF-8 bytes.
    # Preserve that exact ordering so each numbered BIN maps to its source image.
    source_files = sorted(args.rom_dir.glob("*.int"), key=lambda path: path.name.encode("utf-8"))

    if len(card_files) != len(source_files):
        raise SystemExit(
            f"card contains {len(card_files)} numbered BIN files; "
            f"source directory contains {len(source_files)} INT files"
        )

    lines = [
        "# Intellicart SD-card catalogue",
        "",
        "This catalogue records the 178-game `INTELLICART` microSD-card image "
        "prepared for the current firmware revision.",
        "",
        "## Development status",
        "",
        "- Every entry currently falls back to the root `0.cfg`; no game-specific "
        "CFG files are on the card.",
        "- Compatibility is **unverified on physical Intellivision hardware** until "
        "the 74x245 transceivers are fitted and live testing is complete.",
        "- Use the *Class* column to choose a deliberate test mix: standard retail "
        "titles first, then diagnostics, prototypes, demos, and system ROMs.",
        "- The *SD filename* is the exact filename currently on the card. The "
        "firmware derives its OLED title/publisher display from this convention.",
        "- Optional local cover thumbnails use `catalog/covers/NNN.png` (or JPG, "
        "JPEG, or WebP), where `NNN` is the catalogue number.",
        "",
        "## Inventory",
        "",
        "| # | Cover | Title | Year | Publisher | Class | ROM size | SD filename |",
        "| ---: | --- | --- | --- | --- | --- | ---: | --- |",
    ]

    for number, (card, source) in enumerate(zip(card_files, source_files), start=1):
        title, year, publisher, category = parse_source_name(source)
        lines.append(
            "| {number} | {cover} | {title} | {year} | {publisher} | {category} | "
            "{size:,} B | `{filename}` |".format(
                number=number,
                cover=cover_cell(covers_dir, number, title),
                title=escape_table(title),
                year=year,
                publisher=escape_table(publisher),
                category=category,
                size=card.stat().st_size,
                filename=card.name,
            )
        )

    lines.extend(
        [
            "",
            "## Updating this catalogue",
            "",
            "Regenerate after changing the card image. The source ROM directory and "
            "card filenames must retain the same alphabetical/numbered ordering used "
            "when the card was prepared.",
            "",
            "```bash",
            "python3 scripts/generate_sd_catalog.py \\",
            "  --card-dir /media/$USER/INTELLICART \\",
            "  --rom-dir ../rom-asm-intellivision/roms \\",
            "  --covers-dir catalog/covers \\",
            "  --output catalog/SD-CARD-CATALOG.md",
            "```",
            "",
        ]
    )

    args.output.write_text("\n".join(lines), encoding="utf-8")


if __name__ == "__main__":
    main()
