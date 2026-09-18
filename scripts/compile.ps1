$ErrorActionPreference = "Stop"

# compila todos los módulos del proyecto y genera MV.exe
& gcc -Wall -Wextra -I.\headers .\main.c .\modules\init.c .\modules\operators.c .\modules\utils.c .\modules\disassembler.c -o .\versions\MV.exe -w

if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

# ejecuta el programa compilado
& .\versions\MV.exe .\vmt\program.vmx
exit $LASTEXITCODE
