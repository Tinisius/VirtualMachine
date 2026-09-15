#include <stdint.h>

int lsh(int number, int shift) {
    // shifting func
    return number << shift;
}

uint16_t highest(uint32_t x) { return (x >> 16) & 0xFFFF; }

uint16_t lowest(uint32_t x) { return x & 0xFFFF; }

int32_t readValue(int8_t mem[], uint8_t operandSizeBytes,
                  uint32_t *physicIndex) {
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
