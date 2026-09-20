MOV EDX, DS
LDL ECX, 1
LDH ECX, 4
MOV EAX, 0x8
SYS 0x1; lee entrada usuario

MOV EAX, [0]; asigna el valor leido
MOV ECX, -1; i = -1

otro: CMP EAX, 0; el valor == 0?
JZ fin
SHR EAX,1; le saca el bit menos significativo
ADD ECX, 1; i++
JMP otro

fin: MOV [4], ECX; pasa a memoria el valor del contador
MOV EDX,DS
LDL ECX, 2
LDH ECX,4
MOV EAX, 0x9
SYS 0x2
STOP