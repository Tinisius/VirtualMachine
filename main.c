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
    uploadMem(argv, machine.memory, &cs_size, machine.registers[CS].value);

    initTableSeg(machine.segments);
    addSegment(machine.segments, 0, cs_size);         // code segment
    addSegment(machine.segments, 1, N_MEM - cs_size); // data segment

    while (corresponds(&machine)) { // analiza si corresponde leer/seguir
                                    // leyendo las instruciones

        int32_t instrLogDir = machine.registers[IP].value;

        // leemos la instruccion
        memRead(machine.registers[IP].value, 1, &machine);
        instruction = machine.registers[MBR].value;

        // separamos tipos y cod operacion
        uint8_t typeB = (instruction >> 6) & 0x03;
        uint8_t typeA = (instruction >> 4) & 0x03;
        uint8_t opC = instruction & 0x1F;

        // guardamos cod en OPC (REGISTRO)
        machine.registers[OPC].value = opC;
        int opIndex = searchOperatorByCode(operators, opC);
        if (opIndex == -1) {
            printf("\nOPERACION INVALIDA\n");
            exit(-1);
        }

        // leemos OPB y guardamos
        int32_t logAdrB = machine.registers[IP].value + 1;
        memRead(logAdrB, typeB, &machine);
        valueB = machine.registers[MBR].value;
        machine.registers[OP2].value =
            ((int32_t)typeB << 24) | (valueB & 0x00FFFFFF);

        if (typeA > 0) {
            // leemos OPA y guardamos
            int32_t logAdrA = logAdrB + typeB;
            memRead(logAdrA, typeA, &machine);
            valueA = machine.registers[MBR].value;
            machine.registers[OP1].value =
                ((int32_t)typeA << 24) | (valueA & 0x00FFFFFF);
        }
        int instrLen = 1 + typeA + typeB;
        if (disassembler)
            disassembleInstruction(
                &machine, obtainPhysicAdr(&machine, instrLogDir), instrLen,
                operators[opIndex].name, typeA, valueA, typeB, valueB);

        // pasamos a la sig instruccion
        machine.registers[IP].value += instrLen;

        // if (opIndex != -1)
        //     printf("OPERACION: %s\n", operators[opIndex].name);
        // printf("instrucion: %0X\n", instruction);
        // printf("TIP0_A: %01x TIPO_B: %01x\n", typeA, typeB);
        // printf("MEM dir: %d \nOPA: %08x OPB: %08x\n",
        //        machine.registers[IP].value, machine.registers[OP1].value,
        //        machine.registers[OP2].value);

        // para operaciones de un operado

        // al pedo, nunca asigna a OP1 ni OP2
        if (typeA == 0) {
            typeA = typeB;
            valueA = valueB;
            typeB = valueB = 0;
        }

        // invocamos la operacion
        operators[opIndex].operation(machine.registers[OP1].value,
                                     machine.registers[OP2].value, &machine);

        printf("\n");
    }

    // for (int i = 0; i < 256; i++) {
    //     printf("%d  ", i);
    //     printBin(machine.memory[i]);
    //     printf("\n");
    // }

    return 0;
}
