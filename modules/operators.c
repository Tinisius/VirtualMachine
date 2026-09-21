#include "../headers/constants.h"
#include "../headers/utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void STOP(int32_t OPA, int32_t OPB, type_machine *m);

void SYS(int32_t OPA, int32_t OPB, type_machine *m) {

    int32_t v_EDX = m->registers[EDX].value; // m->registers[EDX].value; // posicion de memoria
    int32_t v_ECX = m->registers[ECX].value; // m->registers[ECX].value; // cant - tam
    int32_t v_EAX = m->registers[EAX].value; // modo de lectura

    int32_t value;
    int16_t size = highest(v_ECX);
    int16_t numsAmount = lowest(v_ECX);
    int error = 0;
    int32_t valueB = OPB & 0xFFFFFF;

    if (valueB == 1) {                         // READ / LECTURA (escribe en memoria)
        for (int i = 0; i < numsAmount; i++) { // realiza N lecturas
            int32_t logicAdr = v_EDX + i * size;
            printf("\n[%04X]:", obtainPhysicAdr(m, logicAdr));
            if (scanf(" %d", &value) != 1) {
                printf("Entrada invalida\n");
                STOP(0, 0, m);
                break;
            }
            // trunca value en funcion del size, (1 << 8) - 1 es 0xFF
            value = value & ((1ULL << (size * 8)) - 1);

            memWrite(logicAdr, size, m, value,
                     &error); // escribe en memoria y valida
            if (error) {
                printf("error de memoria");
                STOP(0, 0, m);
                break;
            }
        }
    } else if (valueB == 2) { // WRITE / ESCRITURA (lee de memoria)
        for (int i = 0; i < numsAmount; i++) {
            int32_t logicAdr = v_EDX + i * size;
            printf(" [%04X]", obtainPhysicAdr(m, logicAdr));
            memRead(logicAdr, size, m->registers[DS].value, m); // lee de memoria y valida
            if (error) {
                printf("error de memoria");
                STOP(0, 0, m);
                break;
            }
            value = m->registers[MBR].value;
            printFormat(value, v_EAX);
            printf("\n");
        }

    } else { // ERROR
        printf("operando invalido \n");
        STOP(0, 0, m);
    }
}

void JMP(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t OP = getOpType(OPB) ? OPB : OPA;
    uint16_t offset = getOPValue(OP, m) & 0xFFFF;

    // codigo de segmento de CS + desplazamiento
    m->registers[IP].value = (m->registers[CS].value & 0xFFFF0000) | offset;
}

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
    int32_t valA = getOPValue(OPA, m);
    int32_t resultado = ~valA;
    setOPValue(OPA, m, resultado);
    uploadcc(valA, 0, resultado, m, 0);
}

void STOP(int32_t OPA, int32_t OPB, type_machine *m) { m->registers[IP].value = 0xFFFFFFFF; }

void MOV(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t dato = getOPValue(OPB, m);
    setOPValue(OPA, m, dato);
    uploadcc(0, dato, dato, m, 0);
}

void ADD(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t valA = getOPValue(OPA, m);
    int32_t valB = getOPValue(OPB, m);
    // el resultado se calcula en 64 bits para no perder el acarreo al truncar
    int64_t resultado = (int64_t)valA + valB;
    setOPValue(OPA, m, (int32_t)resultado);
    uploadcc(valA, valB, resultado, m, 1);
}

void SUB(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t valA = getOPValue(OPA, m);
    int32_t valB = getOPValue(OPB, m);
    int64_t resultado = (int64_t)valA - valB;
    setOPValue(OPA, m, (int32_t)resultado);
    uploadcc(valA, valB, resultado, m, 2);
}

void MUL(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t valA = getOPValue(OPA, m);
    int32_t valB = getOPValue(OPB, m);
    // el producto exacto entra en 64 bits: asi se ven los bits que no caben en 32
    int64_t resultado = (int64_t)valA * valB;
    setOPValue(OPA, m, (int32_t)resultado);

    uploadcc(valA, valB, resultado, m, 3);
}

void DIV(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t valA = getOPValue(OPA, m);
    int32_t valB = getOPValue(OPB, m);

    if (valB == 0) {
        printf("ERROR: DIVISON POR 0\n");
        STOP(0, 0, m);
        return;
    }
    int64_t resultado = (int64_t)valA / valB;
    setOPValue(OPA, m, (int32_t)resultado);
    uploadcc(valA, valB, resultado, m, 3);
}

void CMP(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t valA = getOPValue(OPA, m);
    int32_t valB = getOPValue(OPB, m);
    // el cmp no modifica los registros ni nada
    int64_t resultado = (int64_t)valA - valB;
    uploadcc(valA, valB, resultado, m, 2);
}

void AND(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t valA = getOPValue(OPA, m);
    int32_t valB = getOPValue(OPB, m);
    int32_t resultado = valA & valB;
    setOPValue(OPA, m, resultado);

    uploadcc(valA, valB, resultado, m, 0);
}

void OR(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t valA = getOPValue(OPA, m);
    int32_t valB = getOPValue(OPB, m);
    int32_t resultado = valA | valB;
    setOPValue(OPA, m, resultado);
    uploadcc(valA, valB, resultado, m, 0);
}

void XOR(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t valA = getOPValue(OPA, m);
    int32_t valB = getOPValue(OPB, m);
    int32_t resultado = valA ^ valB;
    setOPValue(OPA, m, resultado);

    uploadcc(valA, valB, resultado, m, 0);
}

void SWAP(int32_t OPA, int32_t OPB, type_machine *m) {
    //
    XOR(OPA, OPB, m);
    XOR(OPB, OPA, m);
    XOR(OPA, OPB, m);
}

void SHL(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t valA = getOPValue(OPA, m);
    int32_t valB = getOPValue(OPB, m);

    // se ensancha a 64 bits ANTES de correr para no perder los bits que salen
    // por la izquierda; correr 32 bits o mas vacia la palabra
    int32_t shift = valB < 0 ? 0 : (valB > 32 ? 32 : valB);
    int64_t value = (int64_t)valA << shift;

    setOPValue(OPA, m, (int32_t)value);
    uploadcc(valA, valB, value, m, 3);
}

void SHR(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t valA = getOPValue(OPA, m);
    int32_t valB = getOPValue(OPB, m);

    // desplazamiento logico: se corre el valor sin signo para que entren ceros
    // por la izquierda; correr 32 bits o mas vacia la palabra
    int32_t value = 0;
    if (valB > 0 && valB < 32)
        value = (uint32_t)valA >> valB;
    else if (valB <= 0)
        value = valA;

    setOPValue(OPA, m, value);
    uploadcc(valA, valB, value, m, 3);
}

void SAR(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t valA = getOPValue(OPA, m);
    int32_t valB = getOPValue(OPB, m);

    // desplazamiento aritmetico: se conserva el bit de signo
    int32_t value = valA >> valB;
    // arShiftRight(valA, valB < 0 ? 0 : (valB > 31 ? 31 : valB));

    printf("\n");
    setOPValue(OPA, m, value);
    uploadcc(valA, valB, value, m, 3);
}

// carga los 2 bytes menos significativos de OPA, con los 2 bytes menos
// significativos de OPB
void LDL(int32_t OPA, int32_t OPB, type_machine *m) {
    int8_t typeA = getOpType(OPA);
    int16_t lowB = lowest(getOPValue(OPB, m));

    int32_t value = (getOPValue(OPA, m) & 0xFFFF0000) | lowB;
    setOPValue(OPA, m, value);
}

// carga los 2 bytes más significativos de OPA, con los 2 bytes menos
// significativos de OPB
void LDH(int32_t OPA, int32_t OPB, type_machine *m) {
    int8_t typeA = getOpType(OPA);
    int16_t lowB = lowest(getOPValue(OPB, m));

    int32_t value = lowest(getOPValue(OPA, m)) | (lowB << 16);

    setOPValue(OPA, m, value);
}

void RND(int32_t OPA, int32_t OPB, type_machine *m) {
    srand(time(NULL));
    int8_t typeA = getOpType(OPA);
    int32_t valB = getOPValue(OPB, m);
    int32_t ran = rand() % (abs(valB) + 1);
    if (valB < 0)
        ran *= -1;

    setOPValue(OPA, m, ran);
}

void TRASH(int32_t OPA, int32_t OPB, type_machine *m) {
    printf("\nERROR OPERACION INVALIDA\n");
    exit(-1);
}
