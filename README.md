# Grupo 0x07

**Trabajo práctico Máquina Virtual - Parte II (2C 2026)**
Fundamentos de la Arquitectura de Computadoras
Universidad Nacional de Mar del Plata - Facultad de Ingeniería

## Descripción

Aplicación en lenguaje C que emula la ejecución de un programa en el lenguaje máquina de una computadora basada en la arquitectura de Andrew Tanenbaum. Desarrollada y compilada en Windows 10 - 11.

El programa a ejecutar se escribe previamente en el lenguaje Assembler de la máquina virtual (provisto por la cátedra) y se traduce a lenguaje máquina con el traductor `vmt` (también provisto por la cátedra).

La máquina virtual puede:

- Ejecutar un programa desde el comienzo (archivo `.vmx`, versiones 1 y 2 de la cabecera).
- Cargar la imagen de un proceso (archivo `.vmi`) y continuar su ejecución.
- Generar imágenes del estado de la máquina en cada breakpoint para depuración.

## Ejecución

1. Descargar la última versión publicada.
2. Generar un archivo `.vmx` con el traductor provisto por la cátedra.
3. Ejecutar `MV_2.0.exe` con los parámetros indicados a continuación.

## Parámetros

```
vmx [filename.vmx] [filename.vmi] [m=M] [-d] [-p param1 param2 ... paramN]
```

| Parámetro | Descripción |
|---|---|
| `filename.vmx` | Ruta al archivo con el programa en lenguaje máquina. |
| `filename.vmi` | Ruta al archivo de imagen de la máquina virtual. |
| `m=M` | *(opcional)* Tamaño de la memoria principal en KiB. Por defecto, 16 KiB. |
| `-d` | *(opcional)* Muestra el código Assembler (desensamblador), incluyendo las constantes del Const Segment y el entry point marcado con `>`. |
| `-p` | *(opcional)* Los argumentos siguientes se pasan como parámetros a la subrutina principal. **Debe ir siempre al final.** |

**Notas:**

- Es obligatorio indicar al menos uno de los dos archivos: `.vmx` y/o `.vmi`.
- Solo `.vmx`: se ejecuta el programa desde el entry point; los breakpoints se ignoran.
- Solo `.vmi`: se carga la memoria, la tabla de segmentos y los registros desde la imagen y se continúa la ejecución. Los parámetros `-p` se ignoran.
- `.vmx` y `.vmi`: se ejecuta el programa y en cada breakpoint se genera la imagen en el archivo `.vmi`.

### Ejemplos

```
MV_2.0.exe programa.vmx m=32 -d -p valor1 valor2
MV_2.0.exe programa.vmx imagen.vmi
MV_2.0.exe imagen.vmi
```

## Depuración

### Breakpoints

Si se indicó un archivo `.vmi`, al ejecutarse un breakpoint (`SYS F`) la máquina virtual pausa la ejecución, genera la imagen y espera una acción del usuario:

| Entrada | Acción |
|---|---|
| `g` (go) | Continúa la ejecución hasta el próximo breakpoint o hasta finalizar. |
| `q` (quit) | Aborta la ejecución, dejando intacto el `.vmi` para poder retomarla luego. |
| `Enter` | Ejecuta la siguiente instrucción y vuelve a hacer un breakpoint (paso a paso). |

### Debugger (provisto por la cátedra)

Monitorea el archivo de imagen y muestra en tiempo real el código desensamblado, los registros y la tabla de segmentos:

```
vmg filename.vmi [-r] [-s] [w=W]
```

- `-r`: muestra los valores de los registros.
- `-s`: muestra la tabla de descriptores de segmentos.
- `w=W`: cantidad de líneas de código a mostrar alrededor del IP.

## Errores detectados

Además de los errores de la Parte I, la máquina virtual informa y aborta la ejecución ante:

- **Memoria insuficiente:** la memoria principal no alcanza para alojar todos los segmentos del proceso.
- **Stack overflow:** `PUSH` o `CALL` sin espacio suficiente en la pila.
- **Stack underflow:** `POP` o `RET` con la pila vacía o sin 4 bytes para extraer.

## Integrantes

- Agustin, Salemme Alonso
- Valentin, Cameranesi
- Camila, Sanchez Bonaldo
