inicio:
    ; Escribimos separados por 4 bytes para no pisarnos[cite: 2]
    MOV [0], 'H'       
    MOV [4], 'o'       
    MOV [8], 'l'       
    MOV [12], 'a'      

    ; Configuramos SYS 2 (WRITE)
    MOV EAX, 0x1F      ; Modo Caracteres[cite: 2]
    MOV EDX, DS        ; Apuntamos al DS[cite: 2]
    
    LDH ECX, 4         ; AHORA Si: Tamaño de la celda de 4 bytes[cite: 2]
    LDL ECX, 4         ; Cantidad a imprimir: 4 valores[cite: 2]
    
    SYS 2              
    STOP