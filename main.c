#include "./headers/operators.h"
#include "headers/disassembler.h"
#include "headers/init.h"
#include "headers/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {
    type_machine machine = {0};
    uint16_t cs_size;
    operatorASM operators[N_OP] = OPERATORS;
    srand(time(NULL));
    int32_t valueA = 0, valueB = 0, instruction;

    type_entry_params params = {
        .vmx_file = NULL,
        .vmi_file = NULL,
        .n_mem = 16384,
        .disassembler = false,
        .optional_params = NULL,
        .n_op_params = 0
    };

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            params.disassembler = true;
        }
        else if (endsWith(argv[i], ".vmx")) {
            params.vmx_file = argv[i];
        }
        else if (endsWith(argv[i], ".vmi")) {
            params.vmi_file = argv[i];
        }
        else if (strncmp(argv[i], "m=", 2) == 0 && argv[i][2] != '\0') {
            int m = atoi(argv[i] + 2);
            if (m > 0)
                params.n_mem = m;
            else
                fatal("Memoria Insuficiente / memoria invalida");
        }
        else if (strcmp(argv[i], "-p") == 0){
            i++;
            params.n_op_params = argc - i;
            params.optional_params = calloc(params.n_op_params, sizeof(char *));
            if (params.n_op_params > 0 && params.optional_params == NULL)
                fatal("NO SE PUDIERON RESERVAR LOS PARAMETROS");
            int param_index = 0;

            while (argv[i]){
                //printf()
                params.optional_params[param_index] = argv[i];
                i++;
                param_index++;
            }
        }
    }

    if (params.vmx_file == NULL)
        fatal("FALTA EL ARCHIVO .vmx. USO: vmx filename.vmx [-d]");

    machine.memory.n_mem = params.n_mem;
    machine.memory.mem_values = calloc(machine.memory.n_mem, sizeof(*machine.memory.mem_values));
    if (machine.memory.mem_values == NULL)
        fatal("NO SE PUDO RESERVAR LA MEMORIA");

    initRegs(machine.registers);
    initMainRegs(machine.registers);
    uploadMem(params.vmx_file, &machine.memory, &cs_size, lowest(machine.registers[CS].value));

    initTableSeg(machine.segments);
    addSegment(machine.segments, 0, cs_size);         // code segment
    addSegment(machine.segments, 1, machine.memory.n_mem - cs_size); // data segment

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
        if (params.disassembler)
            disassembleInstruction(&machine, obtainPhysicAdr(&machine, instrLogDir), instrLen, operators[opC].name, typeA, valueA, typeB, valueB);

        // pasamos a la sig instruccion
        machine.registers[IP].value += instrLen;

        // invocamos la operacion
        operators[opC].operation(machine.registers[OP1].value, machine.registers[OP2].value, &machine);
    }
    // logMachine(machine);

    free(machine.memory.mem_values);
    free(params.optional_params);
    return 0;
}
