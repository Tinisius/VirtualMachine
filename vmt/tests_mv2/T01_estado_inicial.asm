\\DATA 64
\\STACK 64
; ==================================================================
; T01 - ESTADO INICIAL DEL PROCESO (sin -p)
; Ejecutar:  vmx T01_estado_inicial.vmx
; Segmentos presentes: Const, Code, Data, Stack (NO Extra, NO Param)
; Tabla esperada: 0=KS 1=CS 2=DS 3=SS  ->  KS=0x00000000,
;   CS=0x00010000, DS=0x00020000, SS=0x00030000, ES=PS=-1
; Pila al entrar a _start: [SP]=-1 (ret), [SP+4]=argc=0, [SP+8]=argv=-1
;   => SP = SS + 64 - 12 = SS + 52
; Termina con RET (sin STOP): el proceso debe finalizar ahi.
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
L1  EQU "T01.1 [SP] = -1 (direccion de retorno de main)"
L2  EQU "T01.2 [SP+4] = argc = 0"
L3  EQU "T01.3 [SP+8] = argv = -1"
L4  EQU "T01.4 SP inicial = SS + 64 - 12"
L5  EQU "T01.5 KS = 0x00000000 (entrada 0 de la tabla)"
L6  EQU "T01.6 CS = 0x00010000 (entrada 1)"
L7  EQU "T01.7 DS = 0x00020000 (entrada 2)"
L8  EQU "T01.8 SS = 0x00030000 (entrada 3)"
L9  EQU "T01.9 ES = -1 (Extra Segment ausente)"
L10 EQU "T01.10 PS = -1 (Param Segment ausente)"
L11 EQU "T01.11 SP vuelve al valor inicial antes del RET"
S_FIN EQU "Ejecutando RET de main: el programa debe terminar ahora"
S_ERR EQU "ERROR: la VM siguio ejecutando despues del RET de main"

_start: MOV ECX, l[SP]
        MOV l[DS+16], ECX
        MOV ECX, l[SP+4]
        MOV l[DS+20], ECX
        MOV ECX, l[SP+8]
        MOV l[DS+24], ECX
        MOV l[DS+28], SP
        MOV b[DS+0], 10
        MOV b[DS+1], 0
        MOV EFX, 0

        MOV EAX, l[DS+16]
        MOV EBX, -1
        MOV EDX, KS
        ADD EDX, L1
        CALL check

        MOV EAX, l[DS+20]
        MOV EBX, 0
        MOV EDX, KS
        ADD EDX, L2
        CALL check

        MOV EAX, l[DS+24]
        MOV EBX, -1
        MOV EDX, KS
        ADD EDX, L3
        CALL check

        MOV EAX, l[DS+28]
        MOV EBX, SS
        ADD EBX, 52
        MOV EDX, KS
        ADD EDX, L4
        CALL check

        MOV EAX, KS
        MOV EBX, 0
        MOV EDX, KS
        ADD EDX, L5
        CALL check

        MOV EAX, CS
        MOV EBX, 1
        SHL EBX, 16
        MOV EDX, KS
        ADD EDX, L6
        CALL check

        MOV EAX, DS
        MOV EBX, 2
        SHL EBX, 16
        MOV EDX, KS
        ADD EDX, L7
        CALL check

        MOV EAX, SS
        MOV EBX, 3
        SHL EBX, 16
        MOV EDX, KS
        ADD EDX, L8
        CALL check

        MOV EAX, ES
        MOV EBX, -1
        MOV EDX, KS
        ADD EDX, L9
        CALL check

        MOV EAX, PS
        MOV EBX, -1
        MOV EDX, KS
        ADD EDX, L10
        CALL check

        MOV EAX, SP
        MOV EBX, l[DS+28]
        MOV EDX, KS
        ADD EDX, L11
        CALL check

        CALL resum
        MOV EDX, KS
        ADD EDX, S_FIN
        SYS 4
        CALL nl
        RET
        MOV EDX, KS
        ADD EDX, S_ERR
        SYS 4
        CALL nl
        STOP
