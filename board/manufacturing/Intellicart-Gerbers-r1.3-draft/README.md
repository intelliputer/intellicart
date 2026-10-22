# Intellicart Gerbers r1.3 — draft

This package is for **manufacturer fitness review only**. Do not place a board
order from it.

Generated from `board/project/intellicart.kicad_pcb` using KiCad 10.0.7 on
2026-10-22. It contains two-layer Gerbers, separate PTH/NPTH Excellon drills,
drill maps, a drill report, DRC report, and front/rear review previews. Paste
layers are intentionally absent because this is a through-hole assembly.

Ask the manufacturer to check the following:

- 1.60 mm finished thickness against an original Intellivision/Intellivoice
  socket;
- 44 exposed fingers, 22 on each face, at 2.54 mm pitch;
- 1.27 mm × 8.84 mm fingers, 493.98 mm² total selective gold area;
- selective electroplated hard nickel/gold fingers and a 30° insertion-edge
  bevel; and
- the `Edge_Cuts` profile, including the 58.1914 mm tongue width and 68.072 mm
  maximum board width. The standalone profile is 68.072 × 98.908 mm; its
  complete perimeter is intentionally board-level artwork rather than
  footprint-owned geometry, for reliable fabricator profile detection.

The DRC report has 0 unconnected pads and 0 schematic-parity issues. Its 21
remaining findings are library-copy/text-size warnings, not copper-clearance
or connectivity findings. The unresolved J1 electrical review remains a
release blocker: see `../J1-INTERFACE-REVIEW.md`.
