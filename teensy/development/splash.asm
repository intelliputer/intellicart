;; Intellicart startup screen.
;;
;; This is a 10-bit-width Intellivision ROM at $5000.  The Teensy firmware
;; embeds the assembled, zero-padded BIN output in splash.h.

        ROMW    10
        ORG     $5000

        INCLUDE "../../../sdk1600/examples/library/gimini.asm"

;; EXEC-compatible cartridge header.
ROMHDR: BIDECLE ZERO
        BIDECLE ZERO
        BIDECLE MAIN
        BIDECLE ZERO
        BIDECLE ONES
        BIDECLE TITLE
        DECLE   $03C0           ; Run after title; no ECS title or clicks.
ZERO:   DECLE   $0000
        DECLE   $0000           ; Colour-stack mode.
ONES:   DECLE   C_BLU, C_BLU
        DECLE   C_BLU, C_BLU
        DECLE   C_CYN           ; Cyan border.

;; The EXEC supplies the familiar title lead-in.  Keep it, but replace its
;; lower copyright line before returning to the EXEC.
TITLE:  PROC
        BYTE    102, "Intellicart", 0
        BEGIN
        CALL    PRINT.FLS
        DECLE   C_WHT, $2C9
        STRING  "Copr  2026 cartheur"
        BYTE    0
        RETURN
        ENDP

MAIN:   PROC
        BEGIN
        CALL    DRAW_SPLASH
        RETURN                  ; The Teensy exits this ROM on any button.
        ENDP

;; Hardware-safe, typography-first color-stack screen.  Custom GRAM tiles
;; remain in the artwork proofs until their initialization path is validated.
DRAW_SPLASH:
        CALL    CLRSCR

        CALL    PRINT.FLS
        DECLE   C_WHT, $200 + 2*20 + 4
        STRING  "INTELLICART"
        BYTE    0

        CALL    PRINT.FLS
        DECLE   C_YEL, $200 + 6*20 + 3
        STRING  "SELECT A GAME"
        BYTE    0

        CALL    PRINT.FLS
        DECLE   C_GRN, $200 + 9*20 + 4
        STRING  "(<)  (o)  (>)"
        BYTE    0

        RETURN

        INCLUDE "../../../sdk1600/examples/library/print.asm"
        INCLUDE "../../../sdk1600/examples/library/fillmem.asm"
