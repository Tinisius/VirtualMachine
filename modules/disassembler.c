#include "../headers/disassembler.h"
#include <stdio.h>
#include <string.h>

// type 0: sin operando | 1: registro (1 byte) | 2: inmediato (2 bytes) | 3:
// memoria [REG+offset] (3 bytes)
static void formatOperand(char *buf, type_machine *m, uint8_t type, int32_t raw) {
    switch (type) {
    case 0:
        buf[0] = '\0';
        break;
    case 1:
        sprintf(buf, "%s", m->registers[raw & 0x1F].name);
        break;
    case 2:
        // el inmediato son 2 bytes en complemento a 2
        sprintf(buf, "%d", (int16_t)(raw & 0xFFFF));
        break;
    case 3: {
        uint16_t offset = (uint16_t)((raw >> 8) & 0xFFFF);
        sprintf(buf, "[%s%+d]", m->registers[raw & 0x1F].name, offset);
        break;
    }
    default:
        buf[0] = '\0';
    }
}

// imprime 1 instruccion ya decodificada. physicDir es la direccion fisica donde
// arranca la instruccion

void disassembleInstruction(type_machine *m, int32_t physicDir, int instrLen, char *mnem, uint8_t typeA, int32_t valueA, uint8_t typeB, int32_t valueB) {
    printf("[%04X] ", physicDir);
    int col = 7; // ancho de "[XXXX] "
    for (int i = 0; i < instrLen && physicDir + i < N_MEM; i++) {
        printf("%02X ", m->memory[physicDir + i]);
        col += 3;
    }
    // relleno con tabs hasta COL_BYTES para que los | queden alineados
    while (col < COL_BYTES) {
        putchar('\t');
        col = (col / 8 + 1) * 8;
    }
    printf("| %-4s ", mnem);

    char opA[32], opB[32];
    formatOperand(opA, m, typeA, valueA);
    formatOperand(opB, m, typeB, valueB);

    if (opA[0] && opB[0])
        printf("%s,\t%s", opA, opB);
    else if (opA[0])
        printf("%s", opA);
    else if (opB[0])
        printf("%s", opB);

    printf("\n");
}
