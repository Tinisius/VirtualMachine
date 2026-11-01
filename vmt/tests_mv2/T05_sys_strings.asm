\\DATA 256
\\STACK 256
; ==================================================================
; T05 - SYS 0 (CLEAR SCREEN), SYS 3 (STRING READ), SYS 4 (STRING
;       WRITE) y SYS F ignorado sin .vmi            ** INTERACTIVO **
; Ejecutar:  vmx T05_sys_strings.vmx
; Entrada a tipear:
;   1) hola       (lectura sin limite, ECX = -1)
;   2) abcdefgh   (lectura con limite ECX = 5 -> debe guardar "abcde")
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
S_BORR EQU "SI VES ESTA LINEA, SYS 0 NO LIMPIO LA PANTALLA"
S_HOLA EQU "Hola desde el Const Segment (SYS 4)"
S_P1   EQU "Ingrese: hola  y Enter > "
S_P2   EQU "Ingrese: abcdefgh  y Enter > "
S_ECO  EQU "Eco: "
L1  EQU "T05.1 SYS 0 no modifica EAX"
L2  EQU "T05.2 SYS 0 no modifica EBX"
L3  EQU "T05.3 SYS 0 no modifica ECX"
L4  EQU "T05.4 SYS 0 no modifica EDX"
L5  EQU "T05.5 SYS 0 no modifica EEX"
L6  EQU "T05.6 SYS F sin .vmi se ignora (ECX intacto)"
L7  EQU "T05.7 SYS 3 (ECX=-1) guardo 'hola' (b[3]='a')"
L8  EQU "T05.8 SYS 3 (ECX=5) guardo el 5to caracter 'e'"
L9  EQU "T05.9 SYS 3 (ECX=5) no escribio el 6to caracter"

_start: MOV b[DS+0], 10
        MOV b[DS+1], 0
        MOV EFX, 0
        ; buffer DS+32 .. DS+159 en cero
        MOV EEX, DS
        ADD EEX, 32
        MOV ECX, 32
zero:   MOV l[EEX], 0
        ADD EEX, 4
        SUB ECX, 1
        JNZ zero

        MOV EDX, KS
        ADD EDX, S_BORR
        SYS 4
        CALL nl
        MOV EAX, 11
        MOV EBX, 22
        MOV ECX, 33
        MOV EDX, 44
        MOV EEX, 55
        SYS 0
        MOV l[DS+200], EAX
        MOV l[DS+204], EBX
        MOV l[DS+208], ECX
        MOV l[DS+212], EDX
        MOV l[DS+216], EEX

        MOV EDX, KS
        ADD EDX, S_HOLA
        SYS 4
        CALL nl

        MOV EAX, l[DS+200]
        MOV EBX, 11
        MOV EDX, KS
        ADD EDX, L1
        CALL check
        MOV EAX, l[DS+204]
        MOV EBX, 22
        MOV EDX, KS
        ADD EDX, L2
        CALL check
        MOV EAX, l[DS+208]
        MOV EBX, 33
        MOV EDX, KS
        ADD EDX, L3
        CALL check
        MOV EAX, l[DS+212]
        MOV EBX, 44
        MOV EDX, KS
        ADD EDX, L4
        CALL check
        MOV EAX, l[DS+216]
        MOV EBX, 55
        MOV EDX, KS
        ADD EDX, L5
        CALL check

        MOV ECX, 77
        SYS 15
        MOV EAX, ECX
        MOV EBX, 77
        MOV EDX, KS
        ADD EDX, L6
        CALL check

        ; --- lectura sin limite ---
        MOV EDX, KS
        ADD EDX, S_P1
        SYS 4
        MOV EDX, DS
        ADD EDX, 32
        MOV ECX, -1
        SYS 3
        MOV EDX, KS
        ADD EDX, S_ECO
        SYS 4
        MOV EDX, DS
        ADD EDX, 32
        SYS 4
        CALL nl
        MOV EAX, b[DS+35]
        MOV EBX, 97
        MOV EDX, KS
        ADD EDX, L7
        CALL check

        ; --- lectura limitada a 5 ---
        MOV EDX, KS
        ADD EDX, S_P2
        SYS 4
        MOV EDX, DS
        ADD EDX, 96
        MOV ECX, 5
        SYS 3
        MOV EDX, KS
        ADD EDX, S_ECO
        SYS 4
        MOV EDX, DS
        ADD EDX, 96
        SYS 4
        CALL nl
        MOV EAX, b[DS+100]
        MOV EBX, 101
        MOV EDX, KS
        ADD EDX, L8
        CALL check
        MOV EAX, b[DS+101]
        MOV EBX, 0
        MOV EDX, KS
        ADD EDX, L9
        CALL check

        CALL resum
        RET
