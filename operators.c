#include "headers/constants.h"
#include "utils.h"
#include <stdint.h>

void SYS(int32_t OPA, int32_t OPB, type_machine m) {
    //
    int32_t v_EDX = m.registers[13].value; // posicion de memoria
    int32_t v_ECX = m.registers[12].value; // cant - tam
    int32_t v_EAX = m.registers[10].value; // modo de lectura

    int32_t value;
    int16_t size = highest(v_ECX);
    int16_t numsAmount = lowest(v_ECX);

    if (OPA == 1) {                            // READ / LECTURA (escribe en memoria)
        for (int i = 0; i < numsAmount; i++) { // realiza N lecturas
            printf("[XXXX]: ");
            scanf("%d\n", &value);
            value = value & ((1ULL << (size * 8)) - 1); // trunca value en func del size,
                                                        // (1 << 8) - 1 es 0xFF
            if (memWrite(value, v_EDX, size, m))        // escribe en memoria y valida
                return 1;
        }
    } else if (OPA == 2) { // WRITE / ESCRITURA (lee de memoria)
        for (int i = 0; i < numsAmount; i++) {
            printf("[XXXX]: ");
            value = memRead(v_EDX, size, m);
        }
        {
            /* code */
        }

    } else // ERROR
        return 0;
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
    //
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