\\DATA 64
\\STACK 256
; ==================================================================
; T07 - BREAKPOINTS (SYS F) e IMAGEN .vmi           ** INTERACTIVO **
; a) vmx T07_breakpoint_imagen.vmx               -> sin pausas
; b) vmx T07_breakpoint_imagen.vmx T07.vmi       -> 'g' en cada pausa
; c) igual que b) pero 'g' y luego 'q'           -> aborta tras "Paso 2"
;    luego: vmx T07.vmi                          -> debe imprimir "Paso 3"
; d) en la 1ra pausa apretar solo Enter          -> paso a paso
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
S_P1 EQU "Paso 1"
S_P2 EQU "Paso 2"
S_P3 EQU "Paso 3 (fin)"
L1  EQU "T07.1 ECX = 222 al final (registros sobreviven a la imagen)"
L2  EQU "T07.2 l[DS+16] = 111 (memoria sobrevive a la imagen)"

_start: MOV b[DS+0], 10
        MOV b[DS+1], 0
        MOV EFX, 0
        MOV EDX, KS
        ADD EDX, S_P1
        SYS 4
        CALL nl
        MOV ECX, 111
        MOV l[DS+16], ECX
        SYS 15
        MOV EDX, KS
        ADD EDX, S_P2
        SYS 4
        CALL nl
        MOV ECX, 222
        SYS 15
        MOV EDX, KS
        ADD EDX, S_P3
        SYS 4
        CALL nl
        MOV EAX, ECX
        MOV EBX, 222
        MOV EDX, KS
        ADD EDX, L1
        CALL check
        MOV EAX, l[DS+16]
        MOV EBX, 111
        MOV EDX, KS
        ADD EDX, L2
        CALL check
        CALL resum
        RET
