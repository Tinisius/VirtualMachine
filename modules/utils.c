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
        int16_t ipPhysicAdr = obtainPhysicAdr(m, m->registers[IP].value);
        ipPhysicAdr -= base;
        return ipPhysicAdr >= 0 && ipPhysicAdr < size;
    }
}

uint16_t obtainPhysicAdr(type_machine *m, int32_t logicAdr) {
    uint16_t segmIndex = highest(logicAdr);
    if (segmIndex < N_SEG) {
        uint16_t offset = lowest(logicAdr);
        uint16_t base = highest(m->segments[segmIndex]);
        return base + offset;
    } else {
        printf("ERROR: FALLO DE SEGMENTO\n");
        exit(-1);
    }
}

// segment es el valor del registro de segmento (ej: DS = 0x00010000),
// el indice en la tabla de segmentos es su parte alta
int inSegment(int32_t physicAdr, int32_t segment, type_machine *m) {
    uint16_t segmIndex = highest(segment);
    if (segmIndex >= N_SEG || segmIndex == -1)
        return 0;

    int32_t base = highest(m->segments[segmIndex]);
    int32_t size = lowest(m->segments[segmIndex]);

    int result = physicAdr >= base && physicAdr < base + size && physicAdr < N_MEM;
    if (!result) {
        printf("\nERROR FALLO DE SEGMENTO\n");
        exit(-1);
    }

    return physicAdr >= base && physicAdr < N_MEM && physicAdr < base + size;
}

int memWrite(int32_t logicAdr, int16_t size, type_machine *m, int32_t value, int *error) {
    int32_t physicalAdr = obtainPhysicAdr(m, logicAdr);

    m->registers[LAR].value = logicAdr;
    m->registers[MAR].value = (size << 16) | (physicalAdr & 0xFFFF);
    m->registers[MBR].value = value;

    for (int i = 0; i < size; i++) {
        if (inSegment(physicalAdr + i, m->registers[DS].value, m)) {
            m->memory[physicalAdr + i] = ((uint32_t)value >> ((size - i - 1) * 8)) & 0xFF;
        } else {
            *error = 1;
            return 1;
        }
    }
    return 0;
}

void memRead(int32_t logicAdr, int16_t size, int32_t segment, type_machine *m) {
    uint16_t physicalAdr = obtainPhysicAdr(m, logicAdr);

    m->registers[LAR].value = logicAdr;
    m->registers[MAR].value = (size << 16) | (physicalAdr & 0xFFFF);

    uint32_t data = 0;
    for (int i = 0; i < size; i++) {
        // Al leer una instruccion segment = el valor guardado en CS
        // si está leyendo una instrucción no es necesario validar inSegment

        if (segment == m->registers[CS].value || inSegment(physicalAdr + i, segment, m)) { // controlo que este dentro del segmento
            data |= (m->memory[physicalAdr + i]) << ((size - i - 1) * 8);
        } else {
            printf("ERROR: FALLO DE SEGMENTO");
            exit(-1);
        }
    }
    m->registers[MBR].value = data;
}

void printBin(int8_t byte) {
    for (int j = 7; j >= 0; j--) {
        printf("%0x ", (byte >> j) & 0b1); // muestra todo el Code Segment
        // if (j == 4)
        //     printf("");
    }
    printf(" ");
}
int negativeCC(uint32_t cc) { return (cc >> 31) & 0x01; }

int zeroCC(uint32_t cc) { return ((cc << 1) >> 31) & 0x01; }

int carryCC(uint32_t cc) { return ((cc << 2) >> 31) & 0x01; }

int overflowCC(uint32_t cc) { return ((cc << 3) >> 31) & 0x01; }

uint32_t getOPValue(uint32_t op, type_machine *m) {
    uint8_t type_op = getOpType(op);
    // inmediato: 2 bytes en complemento a 2, hay que extender el signo a 32
    int32_t value = (int16_t)(op & 0xFFFF);

    if (type_op != 2) {
        if (type_op == 3) {
            memRead(getOPLogicAdress(op, m), 4, m->registers[DS].value, m);
            value = m->registers[MBR].value;

        } else {
            uint8_t reg = op & 0x1F;
            if (reg >= 0 && reg < N_REG) // no hace falta q maneje si esta en un registro
                value = m->registers[reg].value;
            else {
                printf("ERROR GETOPVALUE");
                exit(-1);
            }
        }
    }

    return value;
}

uint8_t getOpType(uint32_t op) { return (uint8_t)((op >> 24) & 0x00000003); }

uint32_t getOPLogicAdress(uint32_t op, type_machine *m) {
    uint32_t adress = m->registers[op & 0x1F].value; // EJ: DS = 0001 0000 0000 0000
    if (getOpType(op) == 3) {                        // el DS es  00 01 00 00
        adress += (int16_t)((op >> 8) & 0xFFFF);     // offset con signo
    }

    return adress;
}

void setOPValue(uint32_t OP, type_machine *m, int32_t newValue) {
    int8_t typeA = getOpType(OP);
    int error = 0;

    if (typeA == 1) { // registro
        m->registers[OP & 0x1F].value = newValue;
    } else {
        if (typeA == 3) { // op memoria
            // estaba en 16?
            uint32_t logic = getOPLogicAdress(OP, m);
            m->registers[MBR].value = newValue;
            m->registers[LAR].value = logic;
            memWrite(logic, 4, m, newValue, &error);
            if (error) {
                printf("\nERROR DE MEMORIA\n");
                exit(-1);
            }
        } else
            error = 1;
    }
    if (error) {
        printf("ERROR SETOPVALUE");
        exit(-1);
    }
}

// N, Z, C y V son los bits 31, 30, 29 y 28 del CC (el resto es reservado).
//
// result es el resultado de la operacion SIN truncar a 32 bits, porque el modo 3
// necesita ver los bits que no entran en la palabra del procesador.
//
// cc_mode dice de que operacion viene el resultado, porque C y V se calculan
// de manera distinta en cada caso:
//   0 = logica (MOV, AND, OR, XOR, NOT, SWAP): no puede desbordar, C y V van en 0
//   1 = suma (ADD)
//   2 = resta (SUB, CMP): se hace como A + (-B)
//   3 = resultado ancho (MUL, DIV, SHL, SHR, SAR): C y V salen de comparar el
//       resultado real contra los 32 bits disponibles
void uploadcc(int32_t valA, int32_t valB, int64_t result, type_machine *m, int cc_mode) {
    int32_t truncated = (int32_t)result; // lo que realmente queda en el destino
    uint32_t cc = 0;
    if (truncated < 0) // N: el resultado es negativo
        cc |= 0x80000000;

    if (truncated == 0) // Z: el resultado es cero
        cc |= 0x40000000;

    switch (cc_mode) {
    case 1: // suma
        // C: la suma sin signo dio la vuelta, el resultado quedo mas chico que A
        if ((uint32_t)truncated < (uint32_t)valA)
            cc |= 0x20000000;

        // V: los dos operandos tienen igual signo y el resultado sale con el contrario
        if (((valA ^ truncated) & (valB ^ truncated)) < 0)
            cc |= 0x10000000;
        break;

    case 2: // resta, que es la suma A + (-B)
        // C: acarreo de salida de esa suma, o sea que la resta no pidio prestado
        if ((uint32_t)valA >= (uint32_t)valB)
            cc |= 0x20000000;

        // V: los operandos tienen distinto signo y el resultado sale con el signo de B
        // EQUIVALE A: signo(A) != signo(B) && signo(resultado) != signo(A)
        // 10 - (-5) = 15 <- SIGNO DE A / -10 - 5 = -15 <- SIGNO DE A
        // SI NO SE CUMPLE LO ANTERIOR HAY OVERFLOW => SE ROMPIO
        if (((valA ^ valB) & (valA ^ truncated)) < 0)
            cc |= 0x10000000;
        break;

    case 3: // el resultado real puede no entrar en 32 bits
        // C: excede los 32 bits del procesador, ni con signo ni sin el
        // if(result > 0xFFFFFFFFLL || result < -0x80000000LL)
        if ((result >> 32) & 1)
            cc |= 0x20000000;

        // V: lo que quedo truncado no es el resultado real
        if (result != truncated)
            cc |= 0x10000000;
        break;
    }

    m->registers[CC].value = cc;
}