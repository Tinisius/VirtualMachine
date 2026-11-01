\\DATA 64
\\STACK 256
; ==================================================================
; T02 - PUSH / POP (orden big endian, extension de signo, truncado)
; Ejecutar:  vmx T02_push_pop.vmx
; Reproduce el ejemplo del enunciado: EEX=0x1234ABCD, PUSH EX
;   -> pila = FF FF AB CD (0xFFFFABCD), luego POP w[DS+16]
;   -> b[DS+16]=0xAB, b[DS+17]=0xCD, b[DS+18] intacto
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
L1  EQU "T02.1 PUSH resta 4 a SP"
L2  EQU "T02.2 PUSH EX guarda 0xFFFFABCD (extiende signo)"
L3  EQU "T02.3 byte en [SP] = 0xFF (MSB en el tope)"
L4  EQU "T02.4 byte en [SP+3] = 0xCD (LSB)"
L5  EQU "T02.5 POP w[DS+16] -> word = 0xABCD"
L6  EQU "T02.6 b[DS+16] = 0xAB"
L7  EQU "T02.7 b[DS+17] = 0xCD"
L8  EQU "T02.8 b[DS+18] no se modifica (0x77)"
L9  EQU "T02.9 POP suma 4 a SP"
L10 EQU "T02.10 POP no borra el valor de la pila"
L11 EQU "T02.11 PUSH inmediato + POP CL (trunca a 1 byte)"
L12 EQU "T02.12 PUSH CH (0xAB) -> POP EAX = 0xFFFFFFAB"
L13 EQU "T02.13 PUSH b[mem]=0x80 -> 0xFFFFFF80"
L14 EQU "T02.14 PUSH w[mem] (0x1234) -> 0x00001234"
L15 EQU "T02.15 PUSH l[mem] -> 0x1234ABCD"
L16 EQU "T02.16 LIFO: 1er POP = 3"
L17 EQU "T02.17 LIFO: 2do POP = 2"
L18 EQU "T02.18 LIFO: 3er POP = 1"
L19 EQU "T02.19 SP final = SP inicial"

_start: MOV b[DS+0], 10
        MOV b[DS+1], 0
        MOV EFX, 0
        MOV l[DS+8], SP
        MOV EEX, 4660
        SHL EEX, 16
        MOV EX, -21555          ; EEX = 0x1234ABCD

        MOV ECX, SP
        PUSH EX
        MOV EAX, SP
        MOV EBX, ECX
        SUB EBX, 4
        MOV EDX, KS
        ADD EDX, L1
        CALL check

        MOV EAX, l[SP]
        MOV EBX, -21555
        MOV EDX, KS
        ADD EDX, L2
        CALL check

        MOV EAX, b[SP]
        MOV EBX, -1
        MOV EDX, KS
        ADD EDX, L3
        CALL check

        MOV EAX, b[SP+3]
        MOV EBX, -51
        MOV EDX, KS
        ADD EDX, L4
        CALL check

        MOV b[DS+16], 119
        MOV b[DS+17], 119
        MOV b[DS+18], 119
        POP w[DS+16]
        MOV EEX, ECX            ; leer la pila liberada ANTES de
        SUB EEX, 4              ; cualquier CALL (que la pisaria)
        MOV EEX, l[EEX]
        MOV l[DS+40], EEX

        MOV EAX, w[DS+16]
        MOV EBX, -21555
        MOV EDX, KS
        ADD EDX, L5
        CALL check

        MOV EAX, b[DS+16]
        MOV EBX, -85
        MOV EDX, KS
        ADD EDX, L6
        CALL check

        MOV EAX, b[DS+17]
        MOV EBX, -51
        MOV EDX, KS
        ADD EDX, L7
        CALL check

        MOV EAX, b[DS+18]
        MOV EBX, 119
        MOV EDX, KS
        ADD EDX, L8
        CALL check

        MOV EAX, SP
        MOV EBX, ECX
        MOV EDX, KS
        ADD EDX, L9
        CALL check

        MOV EAX, l[DS+40]
        MOV EBX, -21555
        MOV EDX, KS
        ADD EDX, L10
        CALL check

        MOV ECX, 4660
        SHL ECX, 16
        MOV CX, -21555          ; ECX = 0x1234ABCD
        PUSH 375                ; 0x00000177
        POP CL                  ; ECX = 0x1234AB77
        MOV EAX, ECX
        MOV EBX, 4660
        SHL EBX, 16
        MOV BX, -21641          ; 0xAB77
        MOV EDX, KS
        ADD EDX, L11
        CALL check

        PUSH CH                 ; CH = 0xAB
        POP EAX
        MOV EBX, -85
        MOV EDX, KS
        ADD EDX, L12
        CALL check

        MOV b[DS+20], -128
        PUSH b[DS+20]
        POP EAX
        MOV EBX, -128
        MOV EDX, KS
        ADD EDX, L13
        CALL check

        MOV EEX, 4660
        SHL EEX, 16
        MOV EX, -21555
        MOV l[DS+24], EEX
        PUSH w[DS+24]
        POP EAX
        MOV EBX, 4660
        MOV EDX, KS
        ADD EDX, L14
        CALL check

        PUSH l[DS+24]
        POP EAX
        MOV EBX, EEX
        MOV EDX, KS
        ADD EDX, L15
        CALL check

        PUSH 1
        PUSH 2
        PUSH 3
        POP EAX
        MOV EBX, 3
        MOV EDX, KS
        ADD EDX, L16
        CALL check
        POP EAX
        MOV EBX, 2
        MOV EDX, KS
        ADD EDX, L17
        CALL check
        POP EAX
        MOV EBX, 1
        MOV EDX, KS
        ADD EDX, L18
        CALL check

        MOV EAX, SP
        MOV EBX, l[DS+8]
        MOV EDX, KS
        ADD EDX, L19
        CALL check

        CALL resum
        RET
