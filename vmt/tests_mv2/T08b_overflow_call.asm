\\DATA 16
\\STACK 64
; ==================================================================
; T08b - STACK OVERFLOW con CALL (recursion infinita)
; Ejecutar:  vmx T08b_overflow_call.vmx
; Esperado: mensaje de STACK OVERFLOW y fin de ejecucion.
; ==================================================================
S_A EQU "Inicio recursion infinita"
S_B EQU "ERROR: NO DEBERIA VERSE (no detecto STACK OVERFLOW)"

rec:    CALL rec
        RET

_start: MOV EDX, KS
        ADD EDX, S_A
        SYS 4
        CALL rec
        MOV EDX, KS
        ADD EDX, S_B
        SYS 4
        STOP
