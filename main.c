#include "./headers/operators.h"
#include "headers/init.h"
#include "headers/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    type_machine machine;
    uint16_t cs_size;
    operatorASM operators[N_OP] = OPERATORS;

    int error = 0;
    int32_t valueA, valueB, instruction;

    initRegs(machine.registers);
    uploadMem(argv, machine.memory, &cs_size, machine.registers[CS].value);

    initTableSeg(machine.segments);
    addSegment(machine.segments, 0, cs_size);         // code segment
    addSegment(machine.segments, 1, N_MEM - cs_size); // data segment

    while (corresponds(&machine)) { // analiza si corresponde leer/seguir
                                    // leyendo las instruciones
        // leemos la instruccion
        memRead(machine.registers[IP].value, 1, &machine, &instruction, &error);

        // separamos tipos y cod operacion
        uint8_t tipeB = (instruction >> 6) & 0x03;
        uint8_t tipeA = (instruction >> 4) & 0x03;
        uint8_t opC = instruction & 0x1F;

        // guardamos cod en OPC (REGISTRO)
        machine.registers[OPC].value = opC;
        int opIndex = searchOperatorByCode(operators, opC);
        if (opIndex == -1) {
            printf("\nOPERACION INVALIDA\n");
            exit(-1);
        }

        // leemos OPB y guardamos
        int32_t logDirB = machine.registers[IP].value + 1;
        memRead(logDirB, tipeB, &machine, &valueB, &error);
        machine.registers[OP2].value =
            ((int32_t)tipeB << 24) | (valueB & 0x00FFFFFF);

        if (tipeA > 0) {
            // leemos OPA y guardamos
            int32_t logDirA = logDirB + tipeB;
            memRead(logDirA, tipeA, &machine, &valueA, &error);
            machine.registers[OP1].value =
                ((int32_t)tipeA << 24) | (valueA & 0x00FFFFFF);
        }
        // pasamos a la sig instruccion
        machine.registers[IP].value += 1 + tipeA + tipeB;

        if (opIndex != -1)
            printf("OPERACION: %s\n", operators[opIndex].name);
        printf("instrucion: %0X\n", instruction);
        printf("TIP0_A: %01x TIPO_B: %01x\n", tipeA, tipeB);
        printf("MEM dir: %d \nOPA: %08x OPB: %08x\n",
               machine.registers[IP].value, machine.registers[OP1].value,
               machine.registers[OP2].value);

        // para operaciones de un operado
        if (tipeA == 0) {
            tipeA = tipeB;
            valueA = valueB;
            tipeB = valueB = 0;
        }

        // invocamos la operacion
        operators[opIndex].operation(machine.registers[OP1].value,
                                     machine.registers[OP2].value, machine);

        printf("\n");
    }

    for (int i = 0; i < 256; i++) {
        printf("%d  ", i);
        printBin(machine.memory[i]);
        printf("\n");
    }

    return 0;
}
