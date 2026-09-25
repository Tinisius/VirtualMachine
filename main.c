#include "./headers/operators.h"
#include "headers/disassembler.h"
#include "headers/init.h"
#include "headers/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {
    type_machine machine;
    uint16_t cs_size;
    operatorASM operators[N_OP] = OPERATORS;
    uint8_t disassembler = argc > 2 && strcmp(argv[2], "-d") == 0;
    srand(time(NULL));
    int error = 0;
    int32_t valueA = 0, valueB = 0, instruction;

    if (argc < 2)
        fatal("FALTA EL ARCHIVO. USO: vmx filename.vmx [-d]");

    initRegs(machine.registers);
    initMainRegs(machine.registers);
    uploadMem(argv, machine.memory, &cs_size, lowest(machine.registers[CS].value));

    initTableSeg(machine.segments);
    addSegment(machine.segments, 0, cs_size);         // code segment
    addSegment(machine.segments, 1, N_MEM - cs_size); // data segment

    while (corresponds(&machine)) { // analiza si corresponde leer/seguir
                                    // leyendo las instruciones

        int32_t instrLogDir = machine.registers[IP].value;

        // leemos la instruccion (con segment = CS, memRead no modifica LAR, MAR ni MBR)
        instruction = memRead(machine.registers[IP].value, 1, machine.registers[CS].value, &machine);

        // separamos tipos y cod operacion
        uint8_t typeB = (instruction >> 6) & 0x03;
        uint8_t typeA = (instruction >> 4) & 0x03;
        uint8_t opC = instruction & 0x1F; // al ser unsigned nunca sera <0 y al leer 5 bits nunca será mayor que 32 = N_OP

        machine.registers[OPC].value = opC;

        // leemos OPB y guardamos
        int32_t logAdrB = machine.registers[IP].value + 1;
        valueB = memRead(logAdrB, typeB, machine.registers[CS].value, &machine);
        machine.registers[OP2].value = typeB == 0 ? 0 : ((int32_t)typeB << 24) | (valueB & 0x00FFFFFF);

        // leemos OPA y guardamos
        int32_t logAdrA = logAdrB + typeB;
        valueA = memRead(logAdrA, typeA, machine.registers[CS].value, &machine);
        machine.registers[OP1].value = typeA == 0 ? 0 : ((int32_t)typeA << 24) | (valueA & 0x00FFFFFF);

        
        int instrLen = 1 + typeA + typeB;
        if (disassembler)
            disassembleInstruction(&machine, obtainPhysicAdr(&machine, instrLogDir), instrLen, operators[opC].name, typeA, valueA, typeB, valueB);

        // pasamos a la sig instruccion
        machine.registers[IP].value += instrLen;

        // invocamos la operacion
        operators[opC].operation(machine.registers[OP1].value, machine.registers[OP2].value, &machine);
    }
    // logMachine(machine);

    return 0;
}
