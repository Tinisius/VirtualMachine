#include "../headers/disassembler.h"
#include <stdio.h>
#include <string.h>

// tipe 0: sin operando | 1: registro (1 byte) | 2: inmediato (2 bytes) | 3:
// memoria [REG+offset] (3 bytes)
static void formatOperand(char *buf, type_machine *m, uint8_t tipe,
                          int32_t raw) {
    switch (tipe) {
    case 0:
        buf[0] = '\0';
        break;
    case 1:
        sprintf(buf, "%s", m->registers[raw & 0x1F].name);
        break;
    case 2:
        sprintf(buf, "%d", raw);
        break;
    case 3:
        sprintf(buf, "[%s+%d]", m->registers[raw & 0x1F].name,
                (raw >> 8) & 0xFFFF);
        break;
    default:
        buf[0] = '\0';
    }
}

// imprime UNA instruccion ya decodificada por quien la llama (no lee
// memoria ni recorre el code segment: recibe todo lo que necesita por
// parametro).
void disassembleInstruction(type_machine *m, int32_t physicDir, int instrLen,
                            char *mnem, uint8_t tipeA, int32_t valueA,
                            uint8_t tipeB, int32_t valueB) {
    printf("[%04X] ", physicDir);
    for (int i = 0; i < instrLen; i++)
        printf("%02X ", (uint8_t)m->memory[physicDir + i]);
    printf("| %-4s ", mnem);

    char opA[32], opB[32];
    formatOperand(opA, m, tipeA, valueA);
    formatOperand(opB, m, tipeB, valueB);

    if (opA[0] && opB[0])
        printf("%s,\t%s", opA, opB);
    else if (opA[0])
        printf("%s", opA);
    else if (opB[0])
        printf("%s", opB);

    printf("\n");
}
