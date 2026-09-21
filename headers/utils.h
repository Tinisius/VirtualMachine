#include "operators.h"
#include <constants.h>
#include <stdint.h>

#ifndef UTILS_H
#define UTILS_H

uint16_t highest(uint32_t);

uint16_t lowest(uint32_t);

int searchOperatorByCode(operatorASM[], int16_t);

int corresponds(type_machine *);

uint16_t obtainPhysicAdr(type_machine *, int32_t);

int inSegment(int32_t physicAdr, int32_t segment, type_machine *);

int inMem(int32_t physicAdr);

// actualiza N, Z, C y V del registro CC; result viene sin truncar y cc_mode
// indica de que operacion salio (0 logica, 1 suma, 2 resta, 3 resultado ancho)
void uploadcc(int32_t valA, int32_t valB, int64_t result, type_machine *m, int cc_mode);

int memWrite(int32_t, int16_t, type_machine *, int32_t, int *);

void memRead(int32_t, int16_t, int32_t, type_machine *);

void printBin(int8_t);

uint32_t getOPValue(uint32_t op, type_machine *m);

uint32_t getOPLogicAdress(uint32_t op, type_machine *m);

uint8_t getOpType(uint32_t);

void setOPValue(uint32_t OP, type_machine *m, int32_t newValue);

int negativeCC(uint32_t cc);

int zeroCC(uint32_t cc);

int carryCC(uint32_t cc);

int overflowCC(uint32_t cc);

#endif