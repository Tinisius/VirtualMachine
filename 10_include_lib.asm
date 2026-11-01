; libreria incluida: cuadrado(x) -> EAX
cuadrado: push bp
      mov bp, sp
      mov eax, [bp+8]
      mul eax, eax
      mov sp, bp
      pop bp
      ret
