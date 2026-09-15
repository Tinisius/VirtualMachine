#include "utils.h"
#include "headers/constants.h"
#include <stdint.h>

int lsh(int number, int shift) {
    // shifting func
    return number << shift;
}

uint16_t highest(uint32_t x) { return (x >> 16) & 0xFFFF; }

uint16_t lowest(uint32_t x) { return x & 0xFFFF; }

int32_t readValue(int8_t mem[], uint8_t operandSizeBytes, uint32_t *physicIndex) {
    // EL VALOR PUEDE SER NEGATIVO BOLUDO
    if (operandSizeBytes == 0)
        return 0;

    int32_t v;
    v = mem[*physicIndex];
    v <<= 24;
    v >>= 24;
    operandSizeBytes--;
    for (int i = 0; i < operandSizeBytes; i++) {
        v <<= 8;
        v |= (uint8_t)mem[++(*physicIndex)]; // casteo a sin signo para evitar
                                             // arrastrar basura negativa
    }
    (*physicIndex)++;
    return v;
}
uint32_t obtainPhysicDirection(type_machine m, int32_t logicDir) {
    uint16_t segmIndex = highest(logicDir);
    if (segmIndex < N_SEG) {
        uint16_t offset = lowest(logicDir);
        uint16_t base = highest(m.segments[segmIndex]);
        return base + offset;
    } else {
        printf("te pasaste de segmentos\n");
        exit(-1);
    }
}

int inDS(int32_t physicDir, type_machine m) {
    return highest(m.segments[1]) < physicDir && physicDir < lowest(m.segments[1]);
}

int memWrite(int32_t value, int16_t size, int32_t logicDir, type_machine m) {
    int32_t dir = obtainPhysicDirection(m, logicDir);
    for (int i = 0; i < size; i++) {
        if (inDS(dir + i, m))
            m.memory[dir + i] = value; // escribe el byte en mem
        else
            return 1;
    }
    return 0;
}