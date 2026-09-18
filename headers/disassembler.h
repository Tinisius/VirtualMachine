#include "constants.h"

#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

void disassembleInstruction(type_machine *, int32_t physicDir, int instrLen,
                            char *mnem, uint8_t tipeA, int32_t valueA,
                            uint8_t tipeB, int32_t valueB);

#endif
