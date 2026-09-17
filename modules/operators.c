#include "../headers/constants.h"
#include "../headers/utils.h"
#include <stdint.h>
#include <stdio.h>

void STOP(int32_t OPA, int32_t OPB, type_machine m);

void SYS(int32_t OPA, int32_t OPB, type_machine m) {

    int32_t v_EDX = m.registers[EDX].value; // posicion de memoria
    int32_t v_ECX = m.registers[ECX].value; // cant - tam
    int32_t v_EAX = m.registers[EAX].value; // modo de lectura

    int32_t value;
    int16_t size = highest(v_ECX);
    int16_t numsAmount = lowest(v_ECX);

    int error = 0;
    int32_t valueA = OPA & 0xFFFFFF;

    if (valueA == 1) {                         // READ / LECTURA (escribe en memoria)
        for (int i = 0; i < numsAmount; i++) { // realiza N lecturas
            int32_t logicDir = v_EDX + i * size;
            printf("EDX: %X\n", v_EDX);
            printf("[%04X]:", obtainPhysicDirection(m, logicDir));
            scanf("%d\n", &value);
            // trunca value en funcion del size, (1 << 8) - 1 es 0xFF
            value = value & ((1ULL << (size * 8)) - 1);

            memWrite(logicDir, size, m, value, &error); // escribe en memoria y valida
            if (error) {
                printf("error de memoria");
                STOP(0, 0, m);
                break;
            }
        }
    } else if (valueA == 2) { // WRITE / ESCRITURA (lee de memoria)
        for (int i = 0; i < numsAmount; i++) {
            int32_t logicDir = v_EDX + i * size * 8;
            printf("[%04X]", obtainPhysicDirection(m, logicDir));
            memRead(logicDir, size, m, &value, &error); // lee de memoria y valida
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

void JMP(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void JP(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void JN(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void JZ(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void JC(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void JV(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void JNP(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void JNN(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void JNZ(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void NOT(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void STOP(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void MOV(int32_t OPA, int32_t OPB, type_machine m) {
    
}

void ADD(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void SUB(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void MUL(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void DIV(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void CMP(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void AND(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void OR(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void XOR(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void SWAP(int32_t OPA, int32_t OPB, type_machine m) {
    //
    XOR(OPA, OPB, m);
    XOR(OPB, OPA, m);
    XOR(OPA, OPB, m);
}

void SHL(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void SHR(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void SAR(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void LDL(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void LDH(int32_t OPA, int32_t OPB, type_machine m) {
    //
}

void RND(int32_t OPA, int32_t OPB, type_machine m) {
    //
}