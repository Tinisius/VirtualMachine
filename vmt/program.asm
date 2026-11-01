; TEST 9: breakpoints paso a paso
; Correr:  vmx 09_breakpoints_loop.vmx 09.vmi   (y en otra consola: vmg 09.vmi -r -s)
; En cada pausa: Enter = una instruccion, g = seguir, q = salir
; Luego retomar con:  vmx 09.vmi  (debe continuar desde donde quedo)
; Sin el parametro .vmi los sys 0xF deben ignorarse.
\\DATA 16
\\STACK 64
main: mov eax, 0
      sys 0xF
lazo: add eax, 1
      cmp eax, 5
      jnz lazo
      sys 0xF
      stop