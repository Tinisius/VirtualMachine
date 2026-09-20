LDH EAX, -32768     ; parte alta = 0x8000
        LDL EAX, 0          ; EAX = 0x80000000 = -2147483648
        MOV EBX, -1
        DIV EAX, EBX        ; <-- overflow de la division