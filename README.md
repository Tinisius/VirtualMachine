# Grupo 0x07

Trabajo práctico máquina virtual - Parte I (2C 2026)<br/>
Fundamentos de la Arquitectura de Computadoras<br/>
Universidad Nacional de Mar del Plata<br/>
Facultad de Ingeniería<br/>

## Descripción

Aplicación, en lenguaje de programación "C", que emula la ejecución de un programa en el lenguaje máquina de una computadora basado en la arquitectura de Andrew Tanenbaum. Desarrollada y compilada en Windows 10 - 11

El programa a ejecutar se encuentra previamente escrito en el lenguaje Assembler de la máquina virtual (proveído por la cátedra) y traducido a lenguaje máquina con el programa traductor vmt, (tambien proveído por la cátedra).

## Ejecución

1. Descargar la última versión publicada
2. Generar un archivo .vmx con el traductor provisto por la cátedra
3. Ejecutar el archivo VM.exe (con los parámetros indicados a continuacion)

### Parámetros

1. Ruta al archivo `.vmx` [obligatorio]
2. `-d` [opcional]: habilita el desensamblador
3. `m=<bytes>` [opcional]: define el tamaño de memoria (por defecto, 16384 bytes)
4. `-p` [opcional]: indica que los argumentos siguientes son parámetros opcionales

```
versions/MV.exe ruta/al/archivo/program.vmx m=32000 -d -p valor1 valor2
```

## Integrantes

<li>Agustin, Salemme Alonso</li>
<li>Valentin, Cameranesi</li>
<li>Camila, Sanchez Bonaldo</li>
