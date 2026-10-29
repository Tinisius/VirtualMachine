#include "../headers/disassembler.h"
#include "../headers/utils.h"
#include <stdio.h>
#include <string.h>

// type 0: sin operando | 1: registro (1 byte) | 2: inmediato (2 bytes) | 3:
// memoria [REG+offset] (3 bytes)
void formatOperand(char *buf, type_machine *m, uint8_t type, int32_t raw) {
    switch (type) {
    case 0:
        buf[0] = '\0';
        break;
    case 1: {
        uint8_t reg = raw & 0x1F;
        uint8_t sector = (raw >> 5) & 0x03; // 00 EAX | 01 AL | 10 AH | 11 AX
        if (sector == 0 || reg < EAX || reg > EFX)
            sprintf(buf, "%s", m->registers[reg].name);
        else
            sprintf(buf, "%c%c", m->registers[reg].name[1], "LHX"[sector - 1]);
        break;
    }
    case 2:
        // el inmediato son 2 bytes en complemento a 2
        sprintf(buf, "%d", (int16_t)(raw & 0xFFFF));
        break;
    case 3: {
        int16_t offset = (int16_t)((raw >> 8) & 0xFFFF); // offset con signo
        uint8_t mod = (raw >> 5) & 0x07;                 // 001 b | 010 w | 100 l
        char prefix = mod == 1 ? 'b' : mod == 2 ? 'w' : 'l';
        sprintf(buf, "%c[%s%+d]", prefix, m->registers[raw & 0x1F].name, offset);
        break;
    }
    default:
        buf[0] = '\0';
    }
}

// imprime la marca de entry point, la direccion fisica y los len bytes
// (si son mas de 7, los primeros 6 seguidos de ..) y rellena hasta el |
void printBytes(type_machine *m, int32_t physicDir, int len, int isEntry) {
    printf("%c[%04X] ", isEntry ? '>' : ' ', physicDir);
    int col = 8; // ancho de " [XXXX] "
    int shown = len > 7 ? 6 : len;
    for (int i = 0; i < shown && physicDir + i < m->memory.n_mem; i++) {
        printf("%02X ", m->memory.mem_values[physicDir + i]);
        col += 3;
    }
    if (shown < len) {
        printf(".. ");
        col += 3;
    }
    // relleno con tabs hasta COL_BYTES para que los | queden alineados
    while (col < COL_BYTES) {
        putchar('\t');
        col = (col / 8 + 1) * 8;
    }
    printf("| ");
}

// imprime 1 instruccion ya decodificada
void disassembleInstruction(type_machine *m, int32_t physicDir, int instrLen, int isEntry, char *mnem, uint8_t typeA, int32_t valueA, uint8_t typeB, int32_t valueB) {
    printBytes(m, physicDir, instrLen, isEntry);
    printf("%-4s ", mnem);

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

// imprime las cadenas del Const Segment y luego el Code Segment completo,
// marcando la instruccion apuntada por IP
void disassemble(type_machine *m, operatorASM operators[]) {
    if (highest(m->registers[KS].value) < N_SEG) {
        uint32_t ks = m->segments[highest(m->registers[KS].value)];
        uint16_t i = highest(ks), end = highest(ks) + lowest(ks);
        while (i < end) {
            uint16_t start = i;
            while (i < end && m->memory.mem_values[i] != '\0')
                i++;
            if (i < end)
                i++; // incluye el '\0'

            printBytes(m, start, i - start, 0);
            putchar('"');
            for (uint16_t j = start; j < i && m->memory.mem_values[j] != '\0'; j++) {
                uint8_t c = m->memory.mem_values[j];
                putchar(c >= 32 && c <= 126 ? c : '.');
            }
            printf("\"\n");
        }
    }

    int32_t cs = m->registers[CS].value;
    uint32_t seg = m->segments[highest(cs)];
    for (uint16_t offset = 0; offset < lowest(seg);) {
        int32_t logicAdr = cs | offset;
        int32_t instruction = memRead(logicAdr, 1, cs, m);
        uint8_t typeB = (instruction >> 6) & 0x03;
        uint8_t typeA = (instruction >> 4) & 0x03;
        int32_t valueB = memRead(logicAdr + 1, typeB, cs, m);
        int32_t valueA = memRead(logicAdr + 1 + typeB, typeA, cs, m);
        int instrLen = 1 + typeA + typeB;

        disassembleInstruction(m, highest(seg) + offset, instrLen, logicAdr == m->registers[IP].value, operators[instruction & 0x1F].name, typeA, valueA, typeB, valueB);
        offset += instrLen;
    }
}
