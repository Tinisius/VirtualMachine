\\DATA 16
\\STACK 16
; ==================================================================
; T09b - STACK UNDERFLOW con RET
; Ejecutar:  vmx T09b_underflow_ret.vmx
; Se vacia la pila con 3 POP y luego RET -> STACK UNDERFLOW.
; ==================================================================
S_A EQU "3 POP OK (pila vacia), ejecutando RET"
S_B EQU "ERROR: NO DEBERIA VERSE (no detecto STACK UNDERFLOW)"

_start: POP EAX
        POP EAX
        POP EAX
        MOV EDX, KS
        ADD EDX, S_A
        SYS 4
        RET
        MOV EDX, KS
        ADD EDX, S_B
        SYS 4
        STOP
