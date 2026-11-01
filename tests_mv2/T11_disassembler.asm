\\DATA 32
\\EXTRA 32
\\STACK 64
; ==================================================================
; T11 - DISASSEMBLER (-d)
; Ejecutar:  vmx T11_disassembler.vmx -d
; Verificar en el listado:
;  * Primero las constantes, formato [XXXX] bytes | "texto":
;      "Hola"         -> 48 6F 6C 61 00            (5 bytes, completo)
;      "abcdef"       -> 61 62 63 64 65 66 00      (7 bytes, completo)
;      "Fundamentos"  -> 46 75 6E 64 61 6D ..      (12 bytes: 6 + ..)
;  * '>' solo en la linea de _start (NO en la primera instruccion)
;  * Sectores y modificadores: AH, CX, EL, AX, b[...], w[...], l[...]
;  * PUSH / POP / CALL / RET desensamblados con su mnemonico
; ==================================================================
S1 EQU "Hola"
S2 EQU "abcdef"
S3 EQU "Fundamentos"

sub:    RET

_start: MOV EDX, DS
        MOV b[DS+0], AH
        MOV CX, w[EDX+0]
        MOV w[DS+2], l[DS+10]
        SHL EL, 1
        PUSH AX
        POP w[ES+4]
        CALL sub
        MOV EDX, KS
        ADD EDX, S1
        SYS 4
        MOV EDX, KS
        ADD EDX, S2
        SYS 4
        MOV EDX, KS
        ADD EDX, S3
        SYS 4
        STOP
