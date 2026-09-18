#include "../headers/constants.h"
#include "../headers/utils.h"
#include <stdint.h>
#include <stdio.h>

void STOP(int32_t OPA, int32_t OPB, type_machine *m);

void SYS(int32_t OPA, int32_t OPB, type_machine *m) {

    int32_t v_EDX = m->registers[27].value;  // m->registers[EDX].value; // posicion de memoria
    int32_t v_ECX = 0x00040002;              // m->registers[ECX].value; // cant - tam
    int32_t v_EAX = m->registers[EAX].value; // modo de lectura

    int32_t value;
    int16_t size = highest(v_ECX);
    int16_t numsAmount = lowest(v_ECX);

    int error = 0;
    int32_t valueB = OPB & 0xFFFFFF;

    if (valueB == 1) {                         // READ / LECTURA (escribe en memoria)
        for (int i = 0; i < numsAmount; i++) { // realiza N lecturas
            int32_t logicDir = v_EDX + i * size;
            printf("[%04X]:", obtainPhysicDirection(m, logicDir));
            if (scanf("%d", &value) != 1) {
                printf("Entrada invalida\n");
                STOP(0, 0, m);
                break;
            }
            // trunca value en funcion del size, (1 << 8) - 1 es 0xFF
            value = value & ((1ULL << (size * 8)) - 1);

            memWrite(logicDir, size, m, value, &error); // escribe en memoria y valida
            if (error) {
                printf("error de memoria");
                STOP(0, 0, m);
                break;
            }

            printf("\n");
        }
    } else if (valueB == 2) { // WRITE / ESCRITURA (lee de memoria)
        for (int i = 0; i < numsAmount; i++) {
            int32_t logicDir = v_EDX + i * size;
            printf("[%04X]", obtainPhysicDirection(m, logicDir));
            memRead(logicDir, size, m, &value, &error); // lee de memoria y valida
            printf(" %d", value);
            if (error) {
                printf("error de memoria");
                STOP(0, 0, m);
                break;
            }
        }

    } else { // ERROR
        printf("operando invalido \n");
        STOP(0, 0, m);
    }
}

void JMP(int32_t OPA, int32_t OPB, type_machine *m) { m->registers[IP].value = m->registers[CS].value + getOPValue(OPA, m); }

void JP(int32_t OPA, int32_t OPB, type_machine *m) {
    if (!negativeCC(m->registers[CC].value) && !zeroCC(m->registers[CC].value))
        JMP(OPA, OPB, m);
}

void JN(int32_t OPA, int32_t OPB, type_machine *m) {
    if (negativeCC(m->registers[CC].value) && !zeroCC(m->registers[CC].value))
        JMP(OPA, OPB, m);
}

void JZ(int32_t OPA, int32_t OPB, type_machine *m) {
    if (!negativeCC(m->registers[CC].value) && zeroCC(m->registers[CC].value))
        JMP(OPA, OPB, m);
}

void JC(int32_t OPA, int32_t OPB, type_machine *m) {
    if (carryCC(m->registers[CC].value))
        JMP(OPA, OPB, m);
}

void JV(int32_t OPA, int32_t OPB, type_machine *m) {
    if (overflowCC(m->registers[CC].value))
        JMP(OPA, OPB, m);
}

void JNP(int32_t OPA, int32_t OPB, type_machine *m) {
    if (negativeCC(m->registers[CC].value) || zeroCC(m->registers[CC].value))
        JMP(OPA, OPB, m);
}

void JNN(int32_t OPA, int32_t OPB, type_machine *m) {
    if (!negativeCC(m->registers[CC].value))
        JMP(OPA, OPB, m);
}

void JNZ(int32_t OPA, int32_t OPB, type_machine *m) {
    if (!zeroCC(m->registers[CC].value))
        JMP(OPA, OPB, m);
}

void NOT(int32_t OPA, int32_t OPB, type_machine *m) {
    //
}

void STOP(int32_t OPA, int32_t OPB, type_machine *m) { m->registers[IP].value = 0xFFFFFFFF; }

void MOV(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t dato = getOPValue(OPB, m);
    setOPValue(OPA,m,dato);
    uploadcc(0,0,dato,m,0);
}

void ADD(int32_t OPA, int32_t OPB, type_machine *m) {
    
}

void SUB(int32_t OPA, int32_t OPB, type_machine *m) {
    //
}

void MUL(int32_t OPA, int32_t OPB, type_machine *m) {
    //
}

void DIV(int32_t OPA, int32_t OPB, type_machine *m) {
    //
}

void CMP(int32_t OPA, int32_t OPB, type_machine *m) {
    //
}

void AND(int32_t OPA, int32_t OPB, type_machine *m) {
    //
}

void OR(int32_t OPA, int32_t OPB, type_machine *m) {
    //
}

void XOR(int32_t OPA, int32_t OPB, type_machine *m) {
    //
}

void SWAP(int32_t OPA, int32_t OPB, type_machine *m) {
    //
    XOR(OPA, OPB, m);
    XOR(OPB, OPA, m);
    XOR(OPA, OPB, m);
}

void SHL(int32_t OPA, int32_t OPB, type_machine *m) {
    int8_t tipeA = getOpType(OPA);
    int error = 0;

    int32_t value = getOPValue(OPA, m) << getOPValue(OPB, m);
    setOPValue(OPA, m, value);
}

void SHR(int32_t OPA, int32_t OPB, type_machine *m) {
    int8_t tipeA = getOpType(OPA);
    int error = 0;

    int32_t value = getOPValue(OPA, m) >> getOPValue(OPB, m);
    setOPValue(OPA, m, value);
}

void SAR(int32_t OPA, int32_t OPB, type_machine *m) {
    int8_t tipeA = getOpType(OPA);

    int32_t value = arShiftRight(getOPValue(OPA, m), getOPValue(OPB, m));
    setOPValue(OPA, m, value);
}

// carga los 2 bytes menos significativos de OPA, con los 2 bytes menos significativos de OPB
void LDL(int32_t OPA, int32_t OPB, type_machine *m) {
    int8_t tipeA = getOpType(OPA);
    int16_t lowB = lowest(getOPValue(OPB, m));

    int32_t value = (getOPValue(OPA, m) & 0xFFFF0000) | lowB;
    setOPValue(OPA, m, value);
}

// carga los 2 bytes más significativos de OPA, con los 2 bytes menos significativos de OPB
void LDH(int32_t OPA, int32_t OPB, type_machine *m) {
    int8_t tipeA = getOpType(OPA);
    int16_t lowB = lowest(getOPValue(OPB, m));

    int32_t value = lowest(getOPValue(OPA, m)) | (lowB << 16);

    setOPValue(OPA, m, value);
}

void RND(int32_t OPA, int32_t OPB, type_machine *m) {
    int8_t tipeA = getOpType(OPA);
    int32_t ran = rand() % (getOPValue(OPB, m) + 1);

    setOPValue(OPA, m, ran);
}
