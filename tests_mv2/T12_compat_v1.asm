; ==================================================================
; T12 - COMPATIBILIDAD CON LA PARTE I (header version 1)
; Traducir con:  vmt T12_compat_v1.asm T12_compat_v1.vmx v=1
; Ejecutar:      vmx T12_compat_v1.vmx
; Esperado: imprime 42 en decimal con SYS 2 y termina con STOP.
; OJO: la convencion de SYS 2 es la de la Parte I. Aca se asume
;      EAX = formato (1 = decimal), EDX = direccion,
;      CL = cantidad de celdas, CH = tamanio de celda.
;      Ajustar a la convencion de su Parte I si es distinta.
; ==================================================================
        MOV EAX, 6
        MUL EAX, 7
        MOV l[DS+0], EAX
        MOV EDX, DS
        MOV EAX, 1
        MOV ECX, 0
        MOV CL, 1
        MOV CH, 4
        SYS 2
        STOP
