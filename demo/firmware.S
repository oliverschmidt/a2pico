.feature c_comments

/*

cl65 firmware.S -C firmware.cfg -o firmware.bin

*/

/*

MIT License

Copyright (c) 2022 Oliver Schmidt (https://a2retro.de/)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/


.repeat 7, count
.scope
slot  = count + 1

.segment .sprintf("io_sel%d",slot)

    ; Restore char output switch
    lda #<$FDF0
    ldx #>$FDF0
    sta $36     ; CSWL
    stx $37     ; CSWH

    ; Access expansion ROM
    lda #$C0 + slot
    sta $07F8   ; MSLOT
    bit $CFFF   ; CLRROM
    jsr banner

    ; Save IRQ vector
    lda $03FE   ; IRQLOCL
    ldx $03FF   ; IRQLOCH
    sta $0478 + slot
    stx $04F8 + slot

    ; Set IRQ vector
    lda #<interrupt
    ldx #>interrupt
    sei
    sta $03FE   ; IRQLOCL
    stx $03FF   ; IRQLOCH
    cli

    ; Try to read char from keyboard
@1: jsr getkey
    bcc @4
    cmp #$1B    ; ESC
    bne @2

    ; Restore IRQ vector
    lda $0478 + slot
    ldx $04F8 + slot
    sei
    sta $03FE   ; IRQLOCL
    stx $03FF   ; IRQLOCH
    cli
    rts

    ; Send char if TX FIFO not full
@2: bit $C081 + slot << 4
    bpl @3
    sta $C080 + slot << 4
    jmp @1

    ; Complain about full TX FIFO
@3: jsr $FF3A   ; BELL
    jmp @1

    ; Receive char if RX FIFO not empty
@4: bit $C081 + slot << 4
    bvc @1
    lda $C080 + slot << 4
    ora #$80
    jsr $FDED   ; COUT
    jmp @1

interrupt:
    lda $07F7
    eor #$80
    sta $07F7
    lda $45
    sta $C081 + slot << 4
    rti

.endscope
.endrep


.segment "io_strb"

copyright:
    .byte $0D, $0D
    .byte "COPYRIGHT (C) 2022", $0D
    .byte "OLIVER SCHMIDT (HTTPS://A2RETRO.DE/)", $0D
    .byte $0D, $00

banner:
    ldx #$00
@1: lda copyright,x
    beq @2
    ora #$80
    jsr $FDED   ; COUT
    inx
    jmp @1
@2: rts

getkey:
    lda $C000
    bpl @2
    bit $C010
    and #$7F
    cmp #$0D    ; CR
    bne @1
    lda #$0A    ; LF
@1: sec
    rts
@2: clc
    rts
