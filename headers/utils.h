#include "operators.h"
#include <constants.h>
#include <stdint.h>

#ifndef UTILS_H
#define UTILS_H

uint16_t highest(uint32_t);

uint16_t lowest(uint32_t);

int searchOperatorByCode(operatorASM[], int16_t);

int corresponds(type_machine *);

int32_t readValue(int8_t[], uint8_t, uint32_t *);

uint32_t obtainPhysicDirection(type_machine *, int32_t);

int inDS(int32_t physicDir, type_machine *);

void uploadcc(int32_t valA, int32_t valB, int32_t result, type_machine *m, int op_mode);

int memWrite(int32_t, int16_t, type_machine *, int32_t, int *);

void memRead(int32_t, int16_t, type_machine *, int32_t *, int *);

void printBin(int8_t);

void memReadValidate(int32_t logicDir,type_machine *m,int size);

uint32_t getOPValue(uint32_t op, type_machine *m);

uint32_t getOPLogicAdress(uint32_t op, type_machine *m);

uint8_t getOpType(uint32_t);

void setOPValue(uint32_t OP, type_machine *m, int32_t newValue);

int negativeCC(uint32_t cc);

int zeroCC(uint32_t cc);

int carryCC(uint32_t cc);

int overflowCC(uint32_t cc);

#endif