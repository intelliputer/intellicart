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
| Finger width | 1.27 mm | J1 copper-pad width |
| Finger plated length | 8.84 mm | J1 copper-pad length, from the insertion edge |
| One-face finger area | 246.99 mm² | 22 × 1.27 mm × 8.84 mm |
| Total selective hard-gold area | 493.98 mm² | both faces; enter 494 mm² unless Eurocircuits directs otherwise |
| Finished board thickness | 1.60 mm nominal | Current KiCad board stackup |
| Surface condition | exposed contacts on both sides | J1 F/B copper pads with mask openings |
| Contact finish | hard gold over nickel | mandatory fabrication requirement |
| Insertion edge | 30° bevel | mandatory fabrication requirement |
| Maximum board width | 68.072 mm | `Edge.Cuts` x extrema |
| Overall board length | 98.908 mm | `Edge.Cuts` y extrema |
| Tongue width | 58.1914 mm | width between the two straight tongue sides |

The previous approximate overall-width, connector-tail-pitch, and generic
"verify" notes were removed: they are not manufacturing controls for this
PCB. Before release, verify these source measurements against the regenerated
copper/profile previews, a known-good cartridge, and the fabricator's detected
profile. Confirm that the 494 mm² value is interpreted as both outer faces in
the Eurocircuits order flow.

See [manufacturing/README.md](manufacturing/README.md) for the release gate.
