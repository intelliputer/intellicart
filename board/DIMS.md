# Cartridge-edge interface dimensions

J1 is an integral 44-contact PCB card edge, not a soldered connector. This
file records the controlled interface facts; the KiCad PCB and a freshly
generated profile preview remain the authority for the complete board outline.

| Feature | Controlled value | Basis |
| --- | ---: | --- |
| Electrical contacts | 44 | 22 contacts on each PCB side |
| Contact pitch | 2.54 mm (0.100 in) | J1 footprint |
| Pitch intervals | 21 | 22 contacts per side |
| First-to-last contact-centre span | 53.34 mm (2.100 in) | 21 × 2.54 mm |
| Finished board thickness | 1.60 mm nominal | Current KiCad board stackup |
| Surface condition | exposed contacts on both sides | J1 F/B copper pads with mask openings |
| Contact finish | hard gold over nickel | mandatory fabrication requirement |
| Insertion edge | bevelled | mandatory fabrication requirement |

The previous approximate overall-width, connector-tail-pitch, and generic
"verify" notes were removed: they are not manufacturing controls for this
PCB. Before release, verify the actual profile, finger length/width, mask
openings, and bevel direction from the regenerated copper/profile previews,
then record the measured values in that release note.

See [manufacturing/README.md](manufacturing/README.md) for the release gate.
