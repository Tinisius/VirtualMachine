#include "operators.h"
#include <constants.h>
#include <stdint.h>


#ifndef UTILS_H
#define UTILS_H

uint16_t highest(uint32_t);

uint16_t lowest(uint32_t);

int searchOperatorByCode(operatorASM[], int16_t);

int corresponds(type_machine);

int32_t readValue(int8_t[], uint8_t, uint32_t *);

uint32_t obtainPhysicDirection(type_machine, int32_t);

int inDS(int32_t physicDir, type_machine m);

int memWrite(int32_t, int16_t, type_machine, int32_t, int *);

void memRead(int32_t, int16_t, type_machine, int32_t *, int *);

void printBin(int8_t);

uint32_t getOPValue(uint32_t, type_machine);

uint8_t getOpType(uint32_t);

uint32_t getOPLogicAdress(uint32_t, type_machine);

#endif