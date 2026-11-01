\\DATA 64
\\STACK 1024
; ==================================================================
; T03 - CALL / RET, subrutinas anidadas, recursion con BP
; Ejecutar:  vmx T03_call_ret.vmx
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
L1  EQU "T03.1 CALL ejecuta la subrutina y RET vuelve"
L2  EQU "T03.2 dentro de la subrutina SP = SP - 4"
L3  EQU "T03.3 [SP] en la subrutina = CS | offset de la instr. siguiente"
L4  EQU "T03.4 SP restaurado despues de RET"
L5  EQU "T03.5 CALL anidado (sub_b llama 2 veces a sub_a)"
L6  EQU "T03.6 CALL con operando registro"
L7  EQU "T03.7 factorial(6) recursivo = 720"
L8  EQU "T03.8 SP balanceado tras la recursion"
L9  EQU "T03.9 BP preservado por la subrutina"

sub_a:  MOV EEX, l[SP]
        MOV l[DS+12], SP
        ADD ECX, 1
        RET

sub_b:  CALL sub_a
        CALL sub_a
        RET

sub_c:  ADD ECX, 10
        RET

; fact(n): parametro en [BP+8], resultado en EAX
fact:   PUSH BP
        MOV BP, SP
        PUSH EBX
        MOV EBX, l[BP+8]
        CMP EBX, 1
        JP fact_r
        MOV EAX, 1
        JMP fact_f
fact_r: SUB EBX, 1
        PUSH EBX
        CALL fact
        ADD SP, 4
        ADD EBX, 1
        MUL EAX, EBX
fact_f: POP EBX
        MOV SP, BP
        POP BP
        RET

_start: MOV b[DS+0], 10
        MOV b[DS+1], 0
        MOV EFX, 0
        MOV ECX, 0
        MOV l[DS+8], SP
        CALL sub_a
r1:     MOV EAX, ECX
        MOV EBX, 1
        MOV EDX, KS
        ADD EDX, L1
        CALL check

        MOV EAX, l[DS+12]
        MOV EBX, l[DS+8]
        SUB EBX, 4
        MOV EDX, KS
        ADD EDX, L2
        CALL check

        MOV EAX, EEX
        MOV EBX, CS
        ADD EBX, r1
        MOV EDX, KS
        ADD EDX, L3
        CALL check

        MOV EAX, SP
        MOV EBX, l[DS+8]
        MOV EDX, KS
        ADD EDX, L4
        CALL check

        MOV ECX, 0
        CALL sub_b
        MOV EAX, ECX
        MOV EBX, 2
        MOV EDX, KS
        ADD EDX, L5
        CALL check

        MOV ECX, 0
        MOV EEX, sub_c
        CALL EEX
        MOV EAX, ECX
        MOV EBX, 10
        MOV EDX, KS
        ADD EDX, L6
        CALL check

        MOV BP, 1234
        PUSH 6
        CALL fact
        ADD SP, 4
        MOV EBX, 720
        MOV EDX, KS
        ADD EDX, L7
        CALL check

        MOV EAX, SP
        MOV EBX, l[DS+8]
        MOV EDX, KS
        ADD EDX, L8
        CALL check

        MOV EAX, BP
        MOV EBX, 1234
        MOV EDX, KS
        ADD EDX, L9
        CALL check

        CALL resum
        RET
