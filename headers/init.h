#include <constants.h>

#ifndef INIT_H
#define INIT_H

void initMainRegs(reg[]);

void initRegSegment(int i, int r, reg regs[]);

void initRegs(reg[]);

void initTableSeg(uint32_t[]);

void addSegment(uint32_t[], uint8_t, uint16_t);

void readHeader(char[], uint16_t *, int8_t *, uint16_t *, uint16_t *, uint16_t *, uint16_t *, uint16_t *, uint8_t *);

void uploadMem(type_machine *, type_entry_params, uint8_t);

void calculateSegments(type_machine *machine, int paraMainSizes, char route[], uint8_t *);

void readVMI(type_machine *, char[]);

#endif