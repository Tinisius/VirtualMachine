#include "headers/operators.h"
#include "headers/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int corresponds(type_machine *m) {
    if (m->registers[IP].value < 0)
        return 0;
    else {
        uint32_t table = m->segments[highest(m->registers[CS].value)];
        uint16_t base = highest(table);
        uint16_t size = lowest(table);
        int16_t ipPhysicDir = obtainPhysicDirection(*m, m->registers[IP].value);
        ipPhysicDir -= base;
        printf("%0x--\n", ipPhysicDir >= 0 && ipPhysicDir < size);
        return ipPhysicDir >= 0 && ipPhysicDir < size;
    }
}

void initRegs(reg regs[]) {
    regs[IP].value = 0;
    regs[CS].value = 0;
    regs[DS].value = 0x00010000;
}

void initTableSeg(int32_t TBS[]) {
    for (int i = 0; i < N_SEG; i++)
        TBS[i] = 0xFFFFFFFF;
}

void addSegment(int32_t TBS[], uint8_t pos, uint16_t size) {
    uint16_t last_size = 0;
    if (pos >= 0 && pos <= 7) {
        if (pos != 0)
            last_size = TBS[pos - 1] & 0x0000FFFF;
        // verificar que entre en memoria?
        TBS[pos] = (int32_t)(last_size << 16) | size;
    } else
        exit(-1);
}

// revisar
void createTableSeg(int32_t TBS[], uint16_t cs_size) {
    TBS[0] = 0;
    TBS[0] |= (cs_size & 0xFFFF);
    TBS[1] = 0;
    TBS[1] |= cs_size;
    TBS[1] = TBS[1] << 16;
    TBS[1] = TBS[1] | (N_MEM - cs_size);

    for (int i = 2; i < N_SEG; i++) {
        TBS[i] = 0xFFFF;
        TBS[i] = TBS[i] << 16;
        TBS[i] = TBS[i] | 0xFFFF;
    }
}

void readHeader(char route[], uint16_t *code_size, int8_t *res) {
    uint8_t line[N_HEADER];
    FILE *arch = fopen(route, "rb");

    if (fread(line, 1, N_HEADER, arch) == N_HEADER) {
        // Bytes 0-4: identificador "VMX26"
        // Byte 5: version
        // Bytes 6-7: sizeano codigo
        *code_size = ((uint16_t)line[6] << 8) | line[7];
        // uso memcmp porque line no es una cadena terminada en \0. comparo
        // byte a byte contra ID
        *res = (memcmp(line, ID, 5) == 0) && (line[5] == VERSION) && ((*code_size) <= N_MEM - 1);

        // TEST: mostrar lectura
        printf("IDENTIFICADOR: \"%.5s\"\n", line);
        printf("VERSION: %d\n", line[5]);
        printf("size EN BYTES: %u\n", *code_size);
    } else
        *res = 0;
    fclose(arch);
}

void uploadMem(char *argv[], int8_t memory[], uint16_t *cs_size, int32_t cs) {
    FILE *arch = fopen(*(argv + 1), "rb"); // abre el archivo indicado por parametro

    uint16_t code_size; // guardamos el sizeaño del code en una var de 2bytes
    int8_t res = 0;     // guarda si es posible ejecutar el programa .vmx

    readHeader(*(argv + 1), &code_size, &res);
    *cs_size = code_size;

    if (res) {
        fseek(arch, N_HEADER, SEEK_SET);

        // aca faltaria inicializar CS
        fread(memory + cs, 1, code_size,
              arch); // guarda en memoria todo el code segment
        fclose(arch);

        for (int i = 0; i < code_size; i++) {
            printBin(memory[i]);
            printf("  (%02x)", (uint8_t)memory[i]);
            printf("\n");
        }
    }
}

int searchOperatorByCode(operatorASM op[], int16_t code) {
    int pri = 0;
    int ult = N_OP - 1;

    while (pri <= ult) {
        int half = pri + (ult - pri) / 2; // Calcula el punto medio exacto

        if (op[half].code == code) {
            return half; // Elemento encontrado, devuelve el índice
        }

        if (code > op[half].code) {
            pri = half + 1; // Busca en la mitad derecha
        } else {
            ult = half - 1; // Busca en la mitad izquierda
        }
    }

    return -1; // No se encontró el código
}

int main(int argc, char *argv[]) {
    type_machine machine;
    uint16_t cs_size;
    operatorASM operators[N_OP] = OPERATORS;

    int error = 0;
    int32_t valueA, valueB, instruction;

    initRegs(machine.registers);
    uploadMem(argv, machine.memory, &cs_size, machine.registers[CS].value);
    //     createTableSeg(machine.segments,
    //                    cs_size); // se crea la tabla de segmentos

    initTableSeg(machine.segments);
    addSegment(machine.segments, 0, cs_size);         // code segment
    addSegment(machine.segments, 1, N_MEM - cs_size); // data segment

    while (corresponds(&machine)) { // analiza si corresponde leer/seguir leyendo las instruciones
        // leemos la instruccion
        memRead(machine.registers[IP].value, 1, machine, &instruction, &error);

        // separamos tipos y cod operacion
        uint8_t tipeB = (instruction >> 6) & 0x03;
        uint8_t tipeA = (instruction >> 4) & 0x03;
        uint8_t opC = instruction & 0x1F;

        // guardamos cod en OPC (REGISTRO)
        machine.registers[OPC].value = opC;
        int opIndex = searchOperatorByCode(operators, opC);

        // leemos OPB y guardamos
        int32_t logDirB = machine.registers[IP].value + 1;
        memRead(logDirB, tipeB, machine, &valueB, &error);
        memRead(machine.registers[IP].value + 8, tipeB + 1, machine, &valueB, &error);
        memRead(machine.registers[IP].value + 8 * (tipeB + 1), tipeB + 1, machine, &valueB, &error);

        machine.registers[OP2].value = ((int32_t)tipeB << 24) | (valueB & 0x00FFFFFF);

        if (tipeA > 0) {
            // leemos OPA y guardamos
            int32_t logDirA = logDirB + tipeB;
            memRead(logDirA, tipeA, machine, &valueA, &error);
            machine.registers[OP1].value = ((int32_t)tipeA << 24) | (valueA & 0x00FFFFFF);
        }
        // pasamos a la sig instruccion
        machine.registers[IP].value += 1 + tipeA + tipeB;

        if (opIndex != -1)
            printf("OPERACION: %s\n", operators[opIndex].name);
        printf("instrucion: %02x\n", instruction);
        printf("TIP0_A: %01x TIPO_B: %01x\n", tipeA, tipeB);
        printf("MEM dir: %d \nOPA: %08x OPB: %08x\n", machine.registers[IP].value,
               machine.registers[OP1].value, machine.registers[OP2].value);

        // para operaciones de un operado
        if (tipeA == 0) {
            tipeA = tipeB;
            valueA = valueB;
            tipeB = valueB = 0;
        }

        // invocamos la operacion
        operators[opIndex].operation(machine.registers[OP1].value, machine.registers[OP2].value,
                                     machine);
    }

    return 0;
}
