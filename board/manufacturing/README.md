# Intellicart fabrication release

## Release status

There is **no current production release**.  `../gerbers/` is retained solely
as an archival KiCad 9.0.2 export from 2026-10-05. The PCB source has a later
filesystem timestamp, so this export cannot be tied conclusively to the
checked-in source; no matching DRC or drill report is present. Treat every
file in that directory as unsuitable for ordering until this procedure is
completed and the resulting revision is reviewed.

This is deliberate release control, not a statement that the design is known
bad.  A board house must receive a package tied to one immutable source
revision and validation result.

## Required board-house specification

- Two copper layers, `F.Cu` and `B.Cu`; source nominal thickness is **1.60 mm**.
  Obtain explicit confirmation that the finished thickness fits the target
  Intellivision connector before ordering. Do not silently substitute 1.55 mm
  or another thickness without recording a PCB revision and regenerating the
  release.
- The board outline is supplied only by the `Edge_Cuts` Gerber. Do not infer a
  profile from copper, solder mask, or silkscreen.
- J1 is the integral 44-contact (2 x 22), 2.54 mm-pitch card edge. Its contacts
  must remain exposed on both sides.
- Specify **hard gold over nickel** on the J1 fingers and a **bevelled insertion
  edge**. ENIG is not an equivalent substitute for a repeatedly inserted card
  edge unless the manufacturer explicitly approves it for this use.
- Do not request paste stencils for this through-hole assembly unless a future
  layout revision introduces paste-requiring components. Exclude `F_Paste` and
  `B_Paste` from the bare-board fabrication archive.

## Production-release procedure

1. Start from a clean working tree. Record the source commit, KiCad version,
   and intended release name, for example `Intellicart-Gerbers-r1.3`.
2. Open `../project/intellicart.kicad_pcb` in the recorded KiCad version.
   Update the board from the schematic if needed, refill zones, and save.
3. Run KiCad DRC with errors, warnings, unconnected items, and footprint errors
   included. Resolve every finding or record each intentional exclusion in the
   report. A release may claim clean status only with **0 errors and 0
   unconnected items**.
4. Generate a drill report. Preserve the plated and non-plated drill counts,
   sizes, and the layer stackup in a text report.
5. Regenerate all fabrication data into a new, revisioned directory; never mix
   it with `../gerbers/`. Include front/back copper, solder mask, silkscreen,
   board profile, PTH/NPTH drills, and the Gerber job file. Generate a front and
   back copper preview and inspect the card-edge contacts, profile, holes, and
   silkscreen clearance manually.
6. Create a ZIP containing exactly the bare-board files plus the DRC and drill
   reports. The ZIP must not contain an earlier release, source files, paste
   layers, or drill-map artwork unless the fabricator requested it.
7. Write a release note with source commit, KiCad version, archive SHA-256,
   DRC result, drill summary, dimensions confirmed from the actual profile, and
   the card-edge finish/bevel instruction. Have a second person inspect the
   ZIP and previews before upload.

## Final order gate

Do not place an order until all are true:

- the revisioned ZIP checksum matches the release note;
- the DRC report and drill report were generated from the same source revision;
- the actual profile and the 44 exposed J1 fingers were visually checked on
  both copper previews;
- the board house acknowledged hard-gold-over-nickel fingers and bevel;
- the selected finished thickness is documented and approved for the console
  connector; and
- a known-good assembled board has passed a console insertion and electrical
  bring-up test.
