\\DATA 64
\\STACK 256
; ==================================================================
; T06 - PARAM SEGMENT, argc / argv
; Ejecutar (desde la carpeta del test, con ESTE nombre exacto):
;   vmx T06_parametros.vmx -p prueba 01 2
; Param Segment esperado (segun figura del enunciado):
;   off 0 "T06_parametros.vmx\0" (19 bytes)  off 19 "prueba\0"
;   off 26 "01\0"   off 29 "2\0"   off 31 argv: 0, 19, 26, 29
; Tabla: 0=PS 1=KS 2=CS 3=DS 4=SS
; OJO: se asume argc = 4 (la figura muestra 4 punteros, incluyendo el
; nombre del .vmx). Si la catedra define argc = 3, cambiar T06.1.
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
L1  EQU "T06.1 argc = 4 (nombre + 3 parametros)"
L2  EQU "T06.2 PS = 0x00000000 (entrada 0, dir. fisica 0)"
L3  EQU "T06.3 KS = 0x00010000 (entrada 1)"
L4  EQU "T06.4 argv: 16 bits altos = 0x0000"
L5  EQU "T06.5 argv: offset = 31"
L6  EQU "T06.6 argv[0] = 0"
L7  EQU "T06.7 argv[1] = 19"
L8  EQU "T06.8 argv[2] = 26"
L9  EQU "T06.9 argv[3] = 29"
L10 EQU "T06.10 b[PS+19] = 'p'"
S_LIS  EQU "Strings apuntados por argv (uno por linea):"

_start: MOV ECX, l[SP+4]
        MOV l[DS+16], ECX
        MOV ECX, l[SP+8]
        MOV l[DS+20], ECX
        MOV b[DS+0], 10
        MOV b[DS+1], 0
        MOV EFX, 0

        MOV EAX, l[DS+16]
        MOV EBX, 4
        MOV EDX, KS
        ADD EDX, L1
        CALL check

        MOV EAX, PS
        MOV EBX, 0
        MOV EDX, KS
        ADD EDX, L2
        CALL check

        MOV EAX, KS
        MOV EBX, 1
        SHL EBX, 16
        MOV EDX, KS
        ADD EDX, L3
        CALL check

        MOV EAX, l[DS+20]
        SHR EAX, 16
        MOV EBX, 0
        MOV EDX, KS
        ADD EDX, L4
        CALL check

        MOV EAX, l[DS+20]
        MOV EBX, 31
        MOV EDX, KS
        ADD EDX, L5
        CALL check

        MOV EEX, l[DS+20]
        MOV EAX, l[EEX]
        MOV EBX, 0
        MOV EDX, KS
        ADD EDX, L6
        CALL check
        MOV EAX, l[EEX+4]
        MOV EBX, 19
        MOV EDX, KS
        ADD EDX, L7
        CALL check
        MOV EAX, l[EEX+8]
        MOV EBX, 26
        MOV EDX, KS
        ADD EDX, L8
        CALL check
        MOV EAX, l[EEX+12]
        MOV EBX, 29
        MOV EDX, KS
        ADD EDX, L9
        CALL check

        MOV EAX, b[PS+19]
        MOV EBX, 112
        MOV EDX, KS
        ADD EDX, L10
        CALL check

        MOV EDX, KS
        ADD EDX, S_LIS
        SYS 4
        CALL nl
        MOV EEX, l[DS+20]
        MOV ECX, l[DS+16]
lazo:   CMP ECX, 0
        JZ fin
        JN fin
        MOV EDX, l[EEX]
        SYS 4
        CALL nl
        ADD EEX, 4
        SUB ECX, 1
        JMP lazo
fin:    CALL resum
        RET
