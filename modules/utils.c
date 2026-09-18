#include "../headers/utils.h"
#include "../headers/constants.h"
#include "../headers/operators.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint16_t highest(uint32_t x) { return (x >> 16) & 0xFFFF; }

uint16_t lowest(uint32_t x) { return x & 0xFFFF; }

int searchOperatorByCode(operatorASM op[], int16_t code) {
    int pri = 0;
    int ult = N_OP - 1;

    while (pri <= ult) {
        int half = pri + (ult - pri) / 2; // Calcula el punto medio exacto

        if (op[half].code == code) {
            return half; // Elemento encontrado, devuelve el índice
        }

        if (code > op[half].code) {
            pri = half + 1; // Busca en la mitad derecha
        } else {
            ult = half - 1; // Busca en la mitad izquierda
        }
    }

    return -1; // No se encontró el código
}

int corresponds(type_machine *m) {
    if (m->registers[IP].value < 0)
        return 0;
    else {
        uint32_t table = m->segments[highest(m->registers[CS].value)];
        uint16_t base = highest(table);
        uint16_t size = lowest(table);
        int16_t ipPhysicDir = obtainPhysicDirection(m, m->registers[IP].value);
        ipPhysicDir -= base;
        return ipPhysicDir >= 0 && ipPhysicDir < size;
    }
}

int32_t readValue(int8_t mem[], uint8_t operandSizeBytes, uint32_t *physicIndex) {
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

uint32_t obtainPhysicDirection(type_machine *m, int32_t logicDir) {
    uint16_t segmIndex = highest(logicDir);
    if (segmIndex < N_SEG) {
        uint16_t offset = lowest(logicDir);
        uint16_t base = highest(m->segments[segmIndex]);
        return base + offset;
    } else {
        printf("te pasaste de segmentos\n");
        exit(-1);
    }
}

int inDS(int32_t physicDir, type_machine *m) {
    int32_t base = highest(m->segments[1]);
    int32_t size = lowest(m->segments[1]);

    return physicDir >= base && physicDir < base + size;
}

int inMem(int32_t physicDir) { return physicDir >= 0 && physicDir < N_MEM; }

int memWrite(int32_t logicDir, int16_t size, type_machine *m, int32_t value, int *error) {
    int32_t dir = obtainPhysicDirection(m, logicDir);
    for (int i = 0; i < size; i++) {
        if (inDS(dir + i, m)) {
            m->memory[dir + i] = value >> (size - i - 1) * 8 & 0xFF; // escribe EL BYTE EN MEM
        } else
            return 1;
    }
    return 0;
}

void memReadValidate(int32_t logicDir,type_machine *m,int size){
    uint16_t physical = obtainPhysicDirection(m,logicDir);
    m->registers[LAR].value = logicDir;
    m->registers[MAR].value = size << 16;
    m->registers[MAR].value = m->registers[MAR].value | physical;
    uint32_t table = m->segments[highest(logicDir)];
    uint16_t limitSeg = highest(table) + lowest(table); //limite de segmento es base y tamaño del mismo
    uint16_t limitAccess = physical + size;
    if( highest(table) <= physical && limitSeg >= limitAccess ){ //controlo que esta en el DS 
        uint32_t data = 0;
            for (int i = 0; i < size; i++)
            data |= ((uint32_t)(uint8_t)m->memory[physical + i]) << ((size - i - 1) * 8);
        m->registers[MBR].value = data;
    }
    else{
        printf("ERROR: FALLO DE SEGMENTO");
        exit(-1);
    }
}


void memRead(int32_t logicDir, int16_t size, type_machine *m, int32_t *value, int *error) {
    int32_t dir = obtainPhysicDirection(m, logicDir);
    *value = 0;
    for (int i = size - 1; i >= 0; i--) {
        if (inMem(dir + i))
            *value += m->memory[dir + i] << (size - i - 1) * 8;
        else {
            *error = 1;
            return;
        }
    }
}

void printBin(int8_t byte) {
    for (int j = 7; j >= 0; j--) {
        printf("%0x ", (byte >> j) & 0b1); // muestra todo el Code Segment
        if (j == 4)
            printf(" ");
    }
}
int negativeCC(uint32_t cc) { return (cc >> 31) & 0x01; }

int zeroCC(uint32_t cc) { return ((cc << 1) >> 31) & 0x01; }

int carryCC(uint32_t cc) { return ((cc << 2) >> 31) & 0x01; }

int overflowCC(uint32_t cc) { return ((cc << 3) >> 31) & 0x01; }

uint32_t getOPValue(uint32_t op, type_machine *m) {
    uint8_t type_op = getOpType(op);
    int32_t value = op & 0x00FFFFFF; 
    int error;

    if (type_op != 2) {
        if (type_op == 3){
            memReadValidate(getOPLogicAdress(op, m),m,4); //----anteriormente memread (sigue existiendo igual)
            value = m->registers[MBR].value;
        }
        else{
            uint8_t reg = op & 0x1F; 
             if (reg >= 0 && reg < N_REG) //no hace falta q maneje si esta en un registro 
                value = m->registers[reg].value;
            else
                error = 1;
        }
    }
    if (error)
        exit(-1);
    return value;
}

uint8_t getOpType(uint32_t op) { return (uint8_t)((op >> 24) & 0x00000003); }

uint32_t getOPLogicAdress(uint32_t op, type_machine *m) {
    uint32_t adress = m->registers[op & 0x1F].value; // EJ: DS = 0001 0000 0000 0000
    if (getOpType(op) == 3) {                        // el DS es  00 01 00 00
        adress += (op >> 8) & 0xFFFF;                
    }

    return adress;
}

void setOPValue(uint32_t OP, type_machine *m, int32_t newValue) {
    int8_t tipeA = getOpType(OP);
    int error = 0;

    if (tipeA == 1) { // registro
        m->registers[OP & 0x1F].value = newValue;
    } else {
        if (tipeA == 3) { // op memoria
            memWrite(getOPLogicAdress(OP, m), 4, m, newValue, &error);
            if (error) {
                STOP(0, 0, m);
                return;
            }
        } else
            error = 1;
    }
    if (error)
        exit(-1);
}

int32_t arShiftRight(int32_t value, int32_t shift) {
    if ((value >> 31) & 0b1) { // si es negativo
        for (int i = 0; i < shift; i++) {
            value = (value >> 1) | (0b1 << 31);
        }
    } else
        return value >> shift;
}