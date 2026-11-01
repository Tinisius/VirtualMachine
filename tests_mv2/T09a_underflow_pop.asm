\\DATA 16
\\STACK 16
; ==================================================================
; T09a - STACK UNDERFLOW con POP
; Ejecutar:  vmx T09a_underflow_pop.vmx
; 3 POP sacan ret, argc y argv (SP = SS+16 -> pila vacia).
; El 4to POP debe dar STACK UNDERFLOW.
; ==================================================================
S_A EQU "3 POP OK (pila vacia)"
S_B EQU "ERROR: NO DEBERIA VERSE (no detecto STACK UNDERFLOW)"

_start: POP EAX
        POP EAX
        POP EAX
        MOV EDX, KS
        ADD EDX, S_A
        SYS 4
        POP EAX
        MOV EDX, KS
        ADD EDX, S_B
        SYS 4
        STOP
