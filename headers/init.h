#include <constants.h>

#ifndef INIT_H
#define INIT_H

void initMainRegs(reg[]);

void initRegs(reg[]);

void initTableSeg(uint32_t[]);

void addSegment(uint32_t[], uint8_t, uint16_t);

void readHeader(char[], uint16_t *, int8_t *);

void uploadMem(char *[], uint8_t[], uint16_t *, int32_t);

#endif