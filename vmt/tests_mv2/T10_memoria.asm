\\DATA 2000
\\STACK 100
; ==================================================================
; T10 - MEMORIA INSUFICIENTE y parametro m=M
; a) vmx T10_memoria.vmx m=1   -> 1 KiB < 2000+100+... -> MEMORIA INSUFICIENTE
; b) vmx T10_memoria.vmx m=2   -> 2 KiB tambien insuficiente
; c) vmx T10_memoria.vmx       -> 16 KiB: imprime el mensaje de OK
; d) vmx T10_memoria.vmx m=64  -> tambien OK
; ==================================================================
S_A EQU "T10 memoria suficiente: proceso cargado -> OK"

_start: MOV EDX, KS
        ADD EDX, S_A
        SYS 4
        RET
