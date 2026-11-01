\\DATA 64
\\STACK 256
; ==================================================================
; T04 - SECTORES DE REGISTRO (AL/AH/AX/EAX) y MODIFICADORES b/w/l
; Ejecutar:  vmx T04_sectores_modificadores.vmx
; ==================================================================
; ------------------------------------------------------------------
; RUTINAS COMUNES (se repiten en cada test; no dependen de SYS 2)
;   check : compara EAX (obtenido) con EBX (esperado).
;           EDX = puntero a la etiqueta (KS + offset del string).
;           Imprime "<etiqueta> -> OK" o "<etiqueta> -> FALLA" y suma
;           1 a EFX (contador de fallas) si no coincide.
;   nl    : imprime un salto de linea (usa b[DS+0]=10, b[DS+1]=0)
;   resum : imprime RESULTADO: TODO OK / HAY FALLAS segun EFX
; Se ubican ANTES de _start para probar que el entry point != 0.
; ------------------------------------------------------------------
S_OK    EQU " -> OK"
S_FAL   EQU " -> FALLA  <<<<<"
S_TOK   EQU "RESULTADO: TODO OK"
S_HAYF  EQU "RESULTADO: HAY FALLAS"

check:  PUSH EDX
        SYS 4
        MOV EDX, KS
        CMP EAX, EBX
        JNZ chk_f
        ADD EDX, S_OK
        JMP chk_p
chk_f:  ADD EDX, S_FAL
        ADD EFX, 1
chk_p:  SYS 4
        CALL nl
        POP EDX
        RET

nl:     PUSH EDX
        MOV EDX, DS
        SYS 4
        POP EDX
        RET

resum:  PUSH EDX
        MOV EDX, KS
        CMP EFX, 0
        JNZ res_f
        ADD EDX, S_TOK
        JMP res_p
res_f:  ADD EDX, S_HAYF
res_p:  SYS 4
        CALL nl
        POP EDX
        RET
; ------------------------------------------------------------------
L1  EQU "T04.1 MOV CL,0 sobre -1 -> 0xFFFFFF00"
L2  EQU "T04.2 MOV CH,0 -> 0xFFFF0000"
L3  EQU "T04.3 MOV CX,0xABCD -> 0xFFFFABCD"
L4  EQU "T04.4 MOV CX,-1 sobre 0 -> 0x0000FFFF"
L5  EQU "T04.5 leer CH=0xFF extiende signo -> -1"
L6  EQU "T04.6 leer CH de 0x1234ABCD -> 0xFFFFFFAB"
L7  EQU "T04.7 leer CL -> 0xFFFFFFCD"
L8  EQU "T04.8 leer CX -> 0xFFFFABCD"
L9  EQU "T04.9 ADD CL,1 solo afecta CL"
L10 EQU "T04.10 MOV EH,1 / MOV EL,2 -> EEX = 0x0102"
L11 EQU "T04.11 b[DS+16] de 0x1234ABCD = 0x12"
L12 EQU "T04.12 b[DS+17] = 0x34"
L13 EQU "T04.13 w[DS+18] = 0xABCD (signo)"
L14 EQU "T04.14 w[DS+17] = 0x34AB"
L15 EQU "T04.15 l[DS+16] = 0x1234ABCD"
L16 EQU "T04.16 MOV b[DS+24],300 guarda 0x2C (trunca)"
L17 EQU "T04.17 MOV w[DS+28],ECX guarda solo 0xABCD"
L18 EQU "T04.18 leer w[DS+28] -> 0xFFFFABCD"
L19 EQU "T04.19 b[EEX+1] con registro base"
L20 EQU "T04.20 MOV w[DS+32],0x1234 -> b[DS+33] = 0x34"

_start: MOV b[DS+0], 10
        MOV b[DS+1], 0
        MOV EFX, 0

        MOV ECX, -1
        MOV CL, 0
        MOV EAX, ECX
        MOV EBX, -256
        MOV EDX, KS
        ADD EDX, L1
        CALL check

        MOV CH, 0
        MOV EAX, ECX
        MOV EBX, -1
        SHL EBX, 16
        MOV EDX, KS
        ADD EDX, L2
        CALL check

        MOV CX, -21555
        MOV EAX, ECX
        MOV EBX, -21555
        MOV EDX, KS
        ADD EDX, L3
        CALL check

        MOV ECX, 0
        MOV CX, -1
        MOV EAX, ECX
        MOV EBX, 1
        SHL EBX, 16
        SUB EBX, 1
        MOV EDX, KS
        ADD EDX, L4
        CALL check

        MOV EAX, CH
        MOV EBX, -1
        MOV EDX, KS
        ADD EDX, L5
        CALL check

        MOV ECX, 4660
        SHL ECX, 16
        MOV CX, -21555          ; ECX = 0x1234ABCD
        MOV EAX, CH
        MOV EBX, -85
        MOV EDX, KS
        ADD EDX, L6
        CALL check

        MOV EAX, CL
        MOV EBX, -51
        MOV EDX, KS
        ADD EDX, L7
        CALL check

        MOV EAX, CX
        MOV EBX, -21555
        MOV EDX, KS
        ADD EDX, L8
        CALL check

        ADD CL, 1
        MOV EAX, ECX
        MOV EBX, 4660
        SHL EBX, 16
        MOV BX, -21554          ; 0xABCE
        MOV EDX, KS
        ADD EDX, L9
        CALL check

        MOV EEX, 0
        MOV EH, 1
        MOV EL, 2
        MOV EAX, EEX
        MOV EBX, 258
        MOV EDX, KS
        ADD EDX, L10
        CALL check

        MOV ECX, 4660
        SHL ECX, 16
        MOV CX, -21555          ; ECX = 0x1234ABCD
        MOV l[DS+16], ECX

        MOV EAX, b[DS+16]
        MOV EBX, 18
        MOV EDX, KS
        ADD EDX, L11
        CALL check

        MOV EAX, b[DS+17]
        MOV EBX, 52
        MOV EDX, KS
        ADD EDX, L12
        CALL check

        MOV EAX, w[DS+18]
        MOV EBX, -21555
        MOV EDX, KS
        ADD EDX, L13
        CALL check

        MOV EAX, w[DS+17]
        MOV EBX, 13483
        MOV EDX, KS
        ADD EDX, L14
        CALL check

        MOV EAX, l[DS+16]
        MOV EBX, ECX
        MOV EDX, KS
        ADD EDX, L15
        CALL check

        MOV l[DS+24], 0
        MOV b[DS+24], 300
        MOV EAX, l[DS+24]
        MOV EBX, 44
        SHL EBX, 24
        MOV EDX, KS
        ADD EDX, L16
        CALL check

        MOV l[DS+28], 0
        MOV w[DS+28], ECX
        MOV EAX, l[DS+28]
        MOV EBX, -21555
        SHL EBX, 16
        MOV EDX, KS
        ADD EDX, L17
        CALL check

        MOV EAX, w[DS+28]
        MOV EBX, -21555
        MOV EDX, KS
        ADD EDX, L18
        CALL check

        MOV EEX, DS
        ADD EEX, 16
        MOV EAX, b[EEX+1]
        MOV EBX, 52
        MOV EDX, KS
        ADD EDX, L19
        CALL check

        MOV w[DS+32], 4660
        MOV EAX, b[DS+33]
        MOV EBX, 52
        MOV EDX, KS
        ADD EDX, L20
        CALL check

        CALL resum
        RET
