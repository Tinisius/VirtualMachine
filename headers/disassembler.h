#include "constants.h"
#include "operators.h"

#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H
#define COL_BYTES 32

void disassemble(type_machine *, operatorASM[]);
void disassembleInstruction(type_machine *, int32_t, int, int, char *, uint8_t, int32_t, uint8_t, int32_t);
void printBytes(type_machine *, int32_t, int, int);
void formatOperand(char *, type_machine *, uint8_t, int32_t);

#endif
