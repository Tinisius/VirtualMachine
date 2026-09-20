#include "constants.h"

#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H
#define COL_BYTES 32

void disassembleInstruction(type_machine *, int32_t physicDir, int instrLen,
                            char *mnem, uint8_t typeA, int32_t valueA,
                            uint8_t typeB, int32_t valueB);

#endif
