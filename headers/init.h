#include <constants.h>

#ifndef INIT_H
#define INIT_H

void initMainRegs(reg[]);

void initRegs(reg[]);

void initTableSeg(int32_t[]);

void addSegment(int32_t[], uint8_t, uint16_t);

void readHeader(char[], uint16_t *, int8_t *);

void uploadMem(char *[], int8_t[], uint16_t *, int32_t);

#endif