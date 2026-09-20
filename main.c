#include "./headers/operators.h"
#include "headers/disassembler.h"
#include "headers/init.h"
#include "headers/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    type_machine machine;
    uint16_t cs_size;
    operatorASM operators[N_OP] = OPERATORS;
    uint8_t disassembler = argc > 2 && strcmp(argv[2], "-d") == 0;
    int error = 0;
    int32_t valueA = 0, valueB = 0, instruction;

    initRegs(machine.registers);
    initMainRegs(machine.registers);
    uploadMem(argv, machine.memory, &cs_size, lowest(machine.registers[CS].value));

    initTableSeg(machine.segments);
    addSegment(machine.segments, 0, cs_size);         // code segment
    addSegment(machine.segments, 1, N_MEM - cs_size); // data segment

    while (corresponds(&machine)) { // analiza si corresponde leer/seguir
                                    // leyendo las instruciones

        int32_t instrLogDir = machine.registers[IP].value;

        // leemos la instruccion
        memRead(machine.registers[IP].value, 1, machine.registers[CS].value, &machine);
        instruction = machine.registers[MBR].value;

        // separamos tipos y cod operacion
        uint8_t typeB = (instruction >> 6) & 0x03;
        uint8_t typeA = (instruction >> 4) & 0x03;
        uint8_t opC = instruction & 0x1F; // al ser unsigned nunca sera <0 y al leer 5 bits nunca será mayor que 32 = N_OP

        machine.registers[OPC].value = opC;

        // leemos OPB y guardamos
        int32_t logAdrB = machine.registers[IP].value + 1;
        memRead(logAdrB, typeB, machine.registers[CS].value, &machine);
        valueB = machine.registers[MBR].value;
        machine.registers[OP2].value = ((int32_t)typeB << 24) | (valueB & 0x00FFFFFF);

        if (typeA > 0) {
            // leemos OPA y guardamos
            int32_t logAdrA = logAdrB + typeB;
            memRead(logAdrA, typeA, machine.registers[CS].value, &machine);
            valueA = machine.registers[MBR].value;
            machine.registers[OP1].value = ((int32_t)typeA << 24) | (valueA & 0x00FFFFFF);
        }
        int instrLen = 1 + typeA + typeB;
        if (disassembler)
            disassembleInstruction(&machine, obtainPhysicAdr(&machine, instrLogDir), instrLen, operators[opC].name, typeA, valueA, typeB, valueB);

        // pasamos a la sig instruccion
        machine.registers[IP].value += instrLen;

        // invocamos la operacion
        operators[opC].operation(machine.registers[OP1].value, machine.registers[OP2].value, &machine);

        printf("\n");
    }
    for (int i = 0; i < N_REG; i++) {
        printf("%s \t", machine.registers[i].name, machine.registers[i].value);
        for (int j = 3; j >= 0; j--) {
            printBin(machine.registers[i].value >> (8 * j));
        }
        printf("\n");
    }
    for (int i = 0; i < 64; i++) {
        printf("%d  ", i);
        printBin(machine.memory[i]);
        printf("\n");
    }

    return 0;
}
