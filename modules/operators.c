#include "../headers/constants.h"
#include "../headers/utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void STOP(int32_t OPA, int32_t OPB, type_machine *m);

void SYS(int32_t OPA, int32_t OPB, type_machine *m) {

    int32_t v_EDX = m->registers[EDX].value; // m->registers[EDX].value; // posicion de memoria
    int32_t v_ECX = m->registers[ECX].value; // m->registers[ECX].value; // cant - tam
    int32_t v_EAX = m->registers[EAX].value; // modo de lectura

    int32_t value;
    uint16_t size = highest(v_ECX);
    uint16_t numsAmount = lowest(v_ECX);
    int32_t call = getOPValue(OPB, m); // numero de llamada al sistema (acepta inmediato, registro o memoria)

    // el tamaño de celda va de 1 a 4 bytes (ademas evita correr 1ULL 64 bits o mas)
    if ((call == 1 || call == 2) && (size < 1 || size > 4))
        fatal("TAMAÑO DE CELDA INVALIDO EN SYS");

    if (call == 1) {                           // READ / LECTURA (escribe en memoria)
        for (int i = 0; i < numsAmount; i++) { // realiza N lecturas
            int32_t logicAdr = v_EDX + i * size;
            printf("[%04X]: ", obtainPhysicAdr(m, logicAdr));
            fflush(stdout); // el prompt no termina en \n: se fuerza antes de leer
            value = readSysValue(v_EAX);

            // trunca value en funcion del size, (1 << 8) - 1 es 0xFF
            value = value & ((1ULL << (size * 8)) - 1);

            memWrite(logicAdr, size, m, value); // escribe en memoria y valida
        }
    } else if (call == 2) { // WRITE / ESCRITURA (lee de memoria)
        for (int i = 0; i < numsAmount; i++) {
            int32_t logicAdr = v_EDX + i * size;
            printf("[%04X]: ", obtainPhysicAdr(m, logicAdr));
            value = memRead(logicAdr, size, m->registers[DS].value, m); // lee de memoria y valida
            printFormat(value, v_EAX, size);
            printf("\n");
        }
    }
}

void JMP(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t OP = getOpType(OPB) ? OPB : OPA;
    uint16_t offset = getOPValue(OP, m) & 0xFFFF;

    // codigo de segmento de CS + desplazamiento. Si el desplazamiento queda fuera
    // del segmento de codigo no es un error: la ejecucion termina (ver corresponds)
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
    int32_t valB = getOPValue(OPB, m);
    int32_t result = ~valB;
    setOPValue(OPB, m, result);
    uploadcc(valB, 0, result, m, 0);
}

void STOP(int32_t OPA, int32_t OPB, type_machine *m) { m->registers[IP].value = -1; }

void MOV(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t value = getOPValue(OPB, m);
    setOPValue(OPA, m, value);
    uploadcc(0, value, value, m, 0);
}

void ADD(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t valA = getOPValue(OPA, m);
    int32_t valB = getOPValue(OPB, m);
    // el resultado se calcula en 64 bits para no perder el acarreo al truncar
    int64_t result = (int64_t)valA + valB;
    setOPValue(OPA, m, (int32_t)result);
    uploadcc(valA, valB, result, m, 1);
}

void SUB(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t valA = getOPValue(OPA, m);
    int32_t valB = getOPValue(OPB, m);
    int64_t result = (int64_t)valA - valB;
    setOPValue(OPA, m, (int32_t)result);
    uploadcc(valA, valB, result, m, 2);
}

void MUL(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t valA = getOPValue(OPA, m);
    int32_t valB = getOPValue(OPB, m);
    // el producto exacto entra en 64 bits: asi se ven los bits que no caben en 32
    int64_t result = (int64_t)valA * valB;
    setOPValue(OPA, m, (int32_t)result);

    uploadcc(valA, valB, result, m, 3);
}

void DIV(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t valA = getOPValue(OPA, m);
    int32_t valB = getOPValue(OPB, m);

    if (valB == 0)
        fatal("DIVISION POR CERO");
    int64_t result = (int64_t)valA / valB;
    setOPValue(OPA, m, (int32_t)result);
    m->registers[AC].value = valA % valB; // el div debe modificar el ac
    uploadcc(valA, valB, result, m, 3);
}

void CMP(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t valA = getOPValue(OPA, m);
    int32_t valB = getOPValue(OPB, m);
    // el cmp no modifica los registros ni nada
    int64_t result = (int64_t)valA - valB;
    uploadcc(valA, valB, result, m, 2);
}

void AND(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t valA = getOPValue(OPA, m);
    int32_t valB = getOPValue(OPB, m);
    int32_t result = valA & valB;
    setOPValue(OPA, m, result);

    uploadcc(valA, valB, result, m, 0);
}

void OR(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t valA = getOPValue(OPA, m);
    int32_t valB = getOPValue(OPB, m);
    int32_t result = valA | valB;
    setOPValue(OPA, m, result);
    uploadcc(valA, valB, result, m, 0);
}

void XOR(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t valA = getOPValue(OPA, m);
    int32_t valB = getOPValue(OPB, m);
    int32_t result = valA ^ valB;
    setOPValue(OPA, m, result);

    uploadcc(valA, valB, result, m, 0);
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
    // se corre sin signo (correr un negativo a la izquierda es UB) y se vuelve a int64
    int64_t value = (int64_t)((uint64_t)(int64_t)valA << shift);

    setOPValue(OPA, m, (int32_t)value);
    uploadcc(valA, valB, value, m, 3);
}

void SHR(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t valA = getOPValue(OPA, m);
    int32_t valB = getOPValue(OPB, m);

    // desplazamiento logico: se corre el valor sin signo para que entren ceros por la izquierda
    // correr 32 bits o mas vacia la palabra
    int32_t value = 0;
    value = (valB > 0) ? (int32_t)((uint32_t)valA >> valB) : valA;

    setOPValue(OPA, m, value);
    uploadcc(valA, valB, value, m, 3);
}

void SAR(int32_t OPA, int32_t OPB, type_machine *m) {
    int32_t valA = getOPValue(OPA, m);
    int32_t valB = getOPValue(OPB, m);

    // desplazamiento aritmetico: se conserva el bit de signo
    // correr 32 bits o mas vacia la palabra
    int32_t value = 0;
    value = (valB > 0) ? valA >> valB : valA;

    setOPValue(OPA, m, value);
    uploadcc(valA, valB, value, m, 3);
}

// carga los 2 bytes menos significativos de OPA, con los 2 bytes menos significativos de OPB
void LDL(int32_t OPA, int32_t OPB, type_machine *m) {
    uint16_t lowB = lowest(getOPValue(OPB, m));

    int32_t value = (getOPValue(OPA, m) & 0xFFFF0000) | lowB;
    setOPValue(OPA, m, value);
}

// carga los 2 bytes más significativos de OPA, con los 2 bytes menos significativos de OPB
void LDH(int32_t OPA, int32_t OPB, type_machine *m) {

    uint16_t lowB = lowest(getOPValue(OPB, m));

    uint32_t value = lowest(getOPValue(OPA, m)) | ((uint32_t)lowB << 16);

    setOPValue(OPA, m, value);
}

void RND(int32_t OPA, int32_t OPB, type_machine *m) {
    int8_t typeA = getOpType(OPA);
    int32_t valB = getOPValue(OPB, m);
    int32_t ran = rand() % (abs(valB) + 1);
    if (valB < 0)
        ran *= -1;

    setOPValue(OPA, m, ran);
}

void POP(int32_t OPA,int32_t OPB,type_machine *m){
     int opSize = sizeObtain(OPA);

    uint16_t seg = (m->registers[SP].value >> 16 ) & 0xFFFF;
    int32_t offset = (int32_t) (m->registers[SP].value & 0xFFFF);
    
    if (offset +4 > lowest(m->segments[seg]) ) 
        fatal("STACK underflow: querias hacer pop de una pila vacia");

    uint32_t dirPhy = highest(m->segments[seg])+(uint32_t)offset;

    uint8_t b0 = m->memory.mem_values[dirPhy];
    uint8_t b1 = m->memory.mem_values[dirPhy+1];
    uint8_t b2 = m->memory.mem_values[dirPhy+2];
    uint8_t b3 = m->memory.mem_values[dirPhy+3];
    int32_t val = ((uint32_t)b0 << 24) | ((uint32_t)b1 << 16) | ((uint32_t)b2 << 8) | (uint32_t)b3;

    if (opSize == 1)
        val = val & 0xFF;
    else
        if(opSize == 2)
             val = val & 0xFFFF;
    
    
    offset+=4; 
    m->registers[SP].value=((uint32_t)seg << 16) | (offset & 0xFFFF);

     setOPValue(OPA,m,val);
    
}

void PUSH(int32_t OPA, int32_t OPB,type_machine *m){
    int opSize = sizeObtain(OPA);
    int32_t val = getOPValue(OPA,m);
    uint16_t seg = (m->registers[SP].value >> 16 ) & 0xFFFF;
    int32_t offset = (int32_t) (m->registers[SP].value & 0xFFFF);
    offset-=4;
    if (offset < 0) 
        fatal("STACK OVERFLOW: Se ha superado el límite del segmento de pila");
    
    m->registers[SP].value=((uint32_t)seg << 16) | (offset & 0xFFFF);

    if (opSize == 1)
        val = (int8_t)val;
    else
        if(opSize == 2)
        val = (int16_t)val;
    uint32_t dirPhy = highest(m->segments[seg])+(uint32_t)offset;
    m->memory.mem_values[dirPhy] = (uint8_t)(val >> 24);
    m->memory.mem_values[dirPhy+1] = (uint8_t)(val >> 16);
    m->memory.mem_values[dirPhy+2] = (uint8_t)(val >> 8);
    m->memory.mem_values[dirPhy+3] = (uint8_t)(val & 0xFF);

}

void CALL(int32_t OPA,int32_t OPB, type_machine *m){
    int32_t val = getOPValue(OPA,m);

    uint16_t seg = (m->registers[SP].value >> 16 ) & 0xFFFF;
    int32_t offset = (int32_t) (m->registers[SP].value & 0xFFFF);

    offset-=4;
    if (offset < 0) 
        fatal("STACK OVERFLOW: Se ha superado el límite del segmento de pila");
    
    m->registers[SP].value=((uint32_t)seg << 16) | (offset & 0xFFFF);

    uint32_t act_ip = m->registers[IP].value;

    uint32_t dirPhy = highest(m->segments[seg])+(uint32_t)offset;
    m->memory.mem_values[dirPhy] = (uint8_t)(act_ip >> 24);
    m->memory.mem_values[dirPhy+1] = (uint8_t)(act_ip >> 16);
    m->memory.mem_values[dirPhy+2] = (uint8_t)(act_ip >> 8);
    m->memory.mem_values[dirPhy+3] = (uint8_t)(act_ip & 0xFF);

    m->registers[IP].value = (m->registers[IP].value & 0xFFFF0000) | ((uint32_t) val &  0xFFFF) ;

}

void RET(int32_t OPA,int32_t OPB, type_machine *m){

    uint16_t seg = (m->registers[SP].value >> 16 ) & 0xFFFF;
    int32_t offset = (int32_t) (m->registers[SP].value & 0xFFFF);

    
    if (offset +4 > lowest(m->segments[seg]) ) 
        fatal("STACK underflow: querias hacer pop de una pila vacia");
    


    uint32_t dirPhy = highest(m->segments[seg])+(uint32_t)offset;

    uint8_t b0 = m->memory.mem_values[dirPhy];
    uint8_t b1 = m->memory.mem_values[dirPhy+1];
    uint8_t b2 = m->memory.mem_values[dirPhy+2];
    uint8_t b3 = m->memory.mem_values[dirPhy+3];
    int32_t val = ((uint32_t)b0 << 24) | ((uint32_t)b1 << 16) | ((uint32_t)b2 << 8) | (uint32_t)b3;

    offset+=4;
    m->registers[SP].value=((uint32_t)seg << 16) | (offset & 0xFFFF);


    m->registers[IP].value = val;

}





//void TRASH(int32_t OPA, int32_t OPB, type_machine *m) { fatal("INSTRUCCION INVALIDA"); }
