$ErrorActionPreference = "Stop"

#compila el main con el nombre MV.exe y lo guarda en versions (-w elimina warnings)
& gcc -Wall -Wextra .\main.c -o .\versions\MV.exe -w

#si la compilacion falló sale (no ejecuta)
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

#y ejecuta el programa compilado
& .\versions\MV.exe .\vmt\program.vmx
exit $LASTEXITCODE
