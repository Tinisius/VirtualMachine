\\DATA 16
\\STACK 16
; ==================================================================
; T08a - STACK OVERFLOW con PUSH
; Ejecutar:  vmx T08a_overflow_push.vmx
; Pila de 16 bytes: al iniciar ocupa 12 (ret, argc, argv) -> SP = SS+4
; 1er PUSH -> SP = SS+0 (OK).  2do PUSH -> SP < SS -> STACK OVERFLOW
; No usa CALL para imprimir (para no gastar pila).
; ==================================================================
S_A EQU "Primer PUSH OK (SP = SS)"
S_B EQU "ERROR: NO DEBERIA VERSE (no detecto STACK OVERFLOW)"

_start: PUSH 1
        MOV EDX, KS
        ADD EDX, S_A
        SYS 4
        PUSH 2
        MOV EDX, KS
        ADD EDX, S_B
        SYS 4
        STOP
