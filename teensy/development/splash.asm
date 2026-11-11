;; Intellicart startup screen.
;;
;; This is a 10-bit-width Intellivision ROM at $5000.  The Teensy firmware
;; embeds the assembled, zero-padded BIN output in splash.h.

        ROMW    10
        ORG     $5000

        INCLUDE "../../../../sdk1600/examples/library/gimini.asm"

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
        DECLE   C_BLU

;; Keep the EXEC's short initial title phase quiet; MAIN draws the screen.
TITLE:  PROC
        BYTE    102, "Intellicart", 0
        RETURN
        ENDP

;; Version 5 of the startup-screen concept, rendered using the built-in GROM
;; font so that every visible element is editable in this source file.
MAIN:   PROC
        CALL    CLRSCR

        ;; Compact cyan/yellow burst from the approved version 5 concept.
        CALL    PRINT.FLS
        DECLE   C_CYN, $200 + 0*20 + 5
        STRING  "/   |   /"
        BYTE    0
        CALL    PRINT.FLS
        DECLE   C_WHT, $200 + 1*20 + 5
        STRING  "INTELLICART"
        BYTE    0
        CALL    PRINT.FLS
        DECLE   C_YEL, $200 + 2*20 + 5
        STRING  "-   |   -"
        BYTE    0

        ;; Intelliputer computer-face mark.
        CALL    PRINT.FLS
        DECLE   C_WHT, $200 + 3*20 + 4
        STRING  ".----------."
        BYTE    0
        CALL    PRINT.FLS
        DECLE   C_WHT, $200 + 4*20 + 4
        STRING  "|  O    O  |"
        BYTE    0
        CALL    PRINT.FLS
        DECLE   C_WHT, $200 + 5*20 + 4
        STRING  "|          |"
        BYTE    0
        CALL    PRINT.FLS
        DECLE   C_WHT, $200 + 6*20 + 4
        STRING  "|  [____]  |"
        BYTE    0
        CALL    PRINT.FLS
        DECLE   C_WHT, $200 + 7*20 + 4
        STRING  "'--|_|_|--'"
        BYTE    0

        CALL    PRINT.FLS
        DECLE   C_YEL, $200 + 8*20 + 3
        STRING  "SELECT A GAME"
        BYTE    0

        ;; Unlabelled previous, select, and next controls.
        CALL    PRINT.FLS
        DECLE   C_GRN, $200 + 10*20 + 2
        STRING  "(<)    (>)    (->)"
        BYTE    0

        RETURN                  ; The Teensy exits this ROM on any button.
        ENDP

        INCLUDE "../../../../sdk1600/examples/library/print.asm"
        INCLUDE "../../../../sdk1600/examples/library/fillmem.asm"
