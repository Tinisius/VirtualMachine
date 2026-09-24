#include "../headers/init.h"
#include "../headers/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void initMainRegs(reg regs[]) {
    regs[IP].value = 0;
    regs[CS].value = 0;
    regs[DS].value = 0x00010000;
}

void initRegs(reg regs[]) {
    char *names[N_REG] = REG_NAMES;

    for (int i = 0; i < N_REG; i++) {
        regs[i].name = names[i];
        regs[i].value = 0;
    }
}

void initTableSeg(uint32_t TBS[]) {
    for (int i = 0; i < N_SEG; i++)
        TBS[i] = 0xFFFFFFFF;
}

void addSegment(uint32_t TBS[], uint8_t pos, uint16_t size) {
    uint16_t last_size = 0;
    if (pos <= 7) {
        if (pos != 0)
            last_size = TBS[pos - 1] & 0x0000FFFF;
        // verificar que entre en memoria?
        TBS[pos] = ((uint32_t)last_size << 16) | size;
    } else
        fatal("NO SE PUDO AGREGAR EL SEGMENTO");
}

void readHeader(char route[], uint16_t *code_size, int8_t *res) {
    uint8_t line[N_HEADER];
    FILE *arch = fopen(route, "rb");
    if (arch == NULL)
        fatal("NO SE PUDO ABRIR EL ARCHIVO");

    if (fread(line, 1, N_HEADER, arch) == N_HEADER) {
        // Bytes 0-4: identificador "VMX26"
        // Byte 5: version
        // Bytes 6-7: sizeano codigo
        *code_size = ((uint16_t)line[6] << 8) | line[7];
        // uso memcmp porque line no es una cadena terminada en \0. comparo
        // byte a byte contra ID
        *res = (memcmp(line, ID, 5) == 0) && (line[5] == VERSION) && ((*code_size) <= N_MEM - 1);

    } else
        *res = 0;
    fclose(arch);
}

void uploadMem(char *argv[], uint8_t memory[], uint16_t *cs_size, int32_t cs) {
    uint16_t code_size; // guardamos el sizeaño del code en una var de 2bytes
    int8_t res = 0;     // guarda si es posible ejecutar el programa .vmx

    readHeader(*(argv + 1), &code_size, &res);
    *cs_size = code_size;

    if (!res)
        fatal("CABECERA INVALIDA: EL ARCHIVO NO ES UN PROGRAMA VMX26 VERSION 1");

    FILE *arch = fopen(*(argv + 1), "rb");
    if (arch == NULL)
        fatal("NO SE PUDO ABRIR EL ARCHIVO");
    fseek(arch, N_HEADER, SEEK_SET);

    // guarda en memoria todo el code segment
    size_t read = fread(memory + cs, 1, code_size, arch);
    fclose(arch);
}