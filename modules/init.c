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

void initTableSeg(int32_t TBS[]) {
    for (int i = 0; i < N_SEG; i++)
        TBS[i] = 0xFFFFFFFF;
}

void addSegment(int32_t TBS[], uint8_t pos, uint16_t size) {
    uint16_t last_size = 0;
    if (pos >= 0 && pos <= 7) {
        if (pos != 0)
            last_size = TBS[pos - 1] & 0x0000FFFF;
        // verificar que entre en memoria?
        TBS[pos] = (int32_t)(last_size << 16) | size;
    } else
        exit(-1);
}

void readHeader(char route[], uint16_t *code_size, int8_t *res) {
    uint8_t line[N_HEADER];
    FILE *arch = fopen(route, "rb");

    if (fread(line, 1, N_HEADER, arch) == N_HEADER) {
        // Bytes 0-4: identificador "VMX26"
        // Byte 5: version
        // Bytes 6-7: sizeano codigo
        *code_size = ((uint16_t)line[6] << 8) | line[7];
        // uso memcmp porque line no es una cadena terminada en \0. comparo
        // byte a byte contra ID
        *res = (memcmp(line, ID, 5) == 0) && (line[5] == VERSION) &&
               ((*code_size) <= N_MEM - 1);

        // TEST: mostrar lectura
        printf("IDENTIFICADOR: \"%.5s\"\n", line);
        printf("VERSION: %d\n", line[5]);
        printf("size EN BYTES: %u\n", *code_size);
    } else
        *res = 0;
    fclose(arch);
}

void uploadMem(char *argv[], int8_t memory[], uint16_t *cs_size, int32_t cs) {
    FILE *arch =
        fopen(*(argv + 1), "rb"); // abre el archivo indicado por parametro

    uint16_t code_size; // guardamos el sizeaño del code en una var de 2bytes
    int8_t res = 0;     // guarda si es posible ejecutar el programa .vmx

    readHeader(*(argv + 1), &code_size, &res);
    *cs_size = code_size;

    if (res) {
        fseek(arch, N_HEADER, SEEK_SET);

        // aca faltaria inicializar CS
        fread(memory + cs, 1, code_size,
              arch); // guarda en memoria todo el code segment
        fclose(arch);

        for (int i = 0; i < code_size; i++) {
            printBin(memory[i]);
            printf("  (%02x)", (uint8_t)memory[i]);
            printf("\n");
        }
    }
}