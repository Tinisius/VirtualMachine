; TEST 7: pseudonimos de registro y modificadores b/w/l (usar con .vmi y breakpoints)
; BP1: EAX = 0x112233FF   (mov al)
; BP2: EAX = 0x1122AAFF   (mov ah)
; BP3: EAX = 0x11225566   (mov ax)
; BP4: DS[0..3] = AA 02 03 04
; BP5: DS[0..3] = AA 02 BB CC
; BP6: ECX = 0xAA02BBCC
; BP7: EBX = 0x00000008   (shl bl,1 no invade los otros bytes; EBX antes = 0x00000004)
\\DATA 16
\\STACK 64

main: 
      ; Corrección para inicializar EAX completo (sortea el límite de 16 bits para inmediatos)
      mov eax, 0x3344
      ldh eax, 0x1122
      
      mov al, 0xFF
      sys 0xF
      
      mov ah, 0xAA
      sys 0xF
      
      mov ax, 0x5566
      sys 0xF
      
      ; Corrección para inicializar la memoria (sortea el límite de 16 bits para inmediatos)[cite: 2]
      ; En lugar de usar un solo MOV con un inmediato de 32 bits (mov l[0], 0x01020304),
      ; cargamos el número completo en un registro auxiliar temporal y luego lo pasamos a la memoria.
      mov eex, 0x0304    ; Carga la parte baja en EEX[cite: 2]
      ldh eex, 0x0102    ; Carga la parte alta en EEX[cite: 2]
      mov l[0], eex      ; Asigna los 4 bytes completos desde el registro a la memoria
      
      mov b[0], 0xAA
      sys 0xF
      
      mov w[2], 0xBBCC
      sys 0xF
      
      mov ecx, l[0]
      sys 0xF
      
      mov ebx, 4
      shl bl, 1
      sys 0xF
      stop