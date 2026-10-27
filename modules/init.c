#include "../headers/init.h"
#include "../headers/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void initMainRegs(reg regs[]) {
    int i;
    for (i = CS; i <= PS; i++)
        regs[i].value = 0xFFFFFFFFF;
}

void initRegSegment(int i, int r, reg regs[]) { regs[r].value = (uint32_t)i << 16; }

void initRegs(reg regs[]) {
    char *names[N_REG] = REG_NAMES;

    for (int i = 0; i < N_REG; i++) {
        regs[i].name = names[i];
        regs[i].value = 0;
    }
}

void initTableSeg(uint32_t TBS[]) {
    for (int i = 0; i < N_SEG; i++)
        TBS[i] = 0xFFFFFFFF;
}

void addSegment(uint32_t TBS[], uint8_t pos, uint16_t size) {
    uint16_t base = 0;
    if (pos <= 7) {
        // el segmento empieza donde termina el anterior: base + tamaño del anterior
        if (pos != 0)
            base = highest(TBS[pos - 1]) + lowest(TBS[pos - 1]);
        // verificar que entre en memoria?
        TBS[pos] = ((uint32_t)base << 16) | size;
    } else
        fatal("NO SE PUDO AGREGAR EL SEGMENTO");
}

void readHeader(char route[], uint16_t *code_size, int8_t *res, uint16_t *ds_size, uint16_t *es_size, uint16_t *ss_size, uint16_t *const_size, uint16_t *entry_point) {
    uint8_t line[N_HEADER]; // N_HEADER AHORA VALE 18 BYTES
    FILE *arch = fopen(route, "rb");
    if (arch == NULL)
        fatal("NO SE PUDO ABRIR EL ARCHIVO");

    if (fread(line, 1, N_HEADER, arch) == N_HEADER) {
        // Bytes 0-4: identificador "VMX26"
        // Byte 5: version
        // Bytes 6-7: sizeano codigo
        *code_size = ((uint16_t)line[6] << 8) | line[7];
        *ds_size = ((uint16_t)line[8] << 8) | line[9];
        *es_size = ((uint16_t)line[10] << 8) | line[11];
        *ss_size = ((uint16_t)line[12] << 8) | line[13];
        *const_size = ((uint16_t)line[14] << 8) | line[15];
        // Bytes 16-17: entry point (offset dentro del code segment)
        *entry_point = ((uint16_t)line[16] << 8) | line[17];

        // uso memcmp porque line no es una cadena terminada en \0. comparo
        // byte a byte contra ID

        *res = (memcmp(line, ID, 5) == 0) && (line[5] == VERSION);

    } else
        *res = 0;
    fclose(arch);
}

void uploadMem(type_machine *mv, type_entry_params params) {

    FILE *arch = fopen(params.vmx_file, "rb");
    if (arch == NULL)
        fatal("NO SE PUDO ABRIR EL ARCHIVO");
    fseek(arch, N_HEADER, SEEK_SET);

    if (params.n_op_params > 0) {
        int32_t ps = mv->registers[PS].value;

        uint32_t strings_size = 0;
        for (int i = 0; i <= params.n_op_params; i++)
            strings_size += strlen(i == 0 ? params.vmx_file : params.optional_params[i - 1]) + 1;

        int32_t argv_adr = ps + strings_size;
        uint32_t offset = 0;
        for (int k = 0; k <= params.n_op_params; k++) {
            char *str = (k == 0) ? params.vmx_file : params.optional_params[k - 1];
            size_t len = strlen(str);

            memWrite(argv_adr + 4 * k, 4, mv, ps + offset);
            for (size_t j = 0; j <= len; j++)
                memWrite(ps + offset + j, 1, mv, str[j]);
            offset += len + 1;
        }
    }

    uint32_t cs = mv->segments[highest(mv->registers[CS].value)];

    if (highest(cs) + lowest(cs) > mv->memory.n_mem)
        fatal("espacio insuficiente");
    if (fread(mv->memory.mem_values + highest(cs), 1, lowest(cs), arch) != lowest(cs))
        fatal("NO SE PUDO LEER EL CODE SEGMENT");

    if (highest(mv->registers[KS].value) < N_SEG) {
        uint32_t ks = mv->segments[highest(mv->registers[KS].value)];
        if (highest(ks) + lowest(ks) > mv->memory.n_mem)
            fatal("espacio insuficiente");
        if (fread(mv->memory.mem_values + highest(ks), 1, lowest(ks), arch) != lowest(ks))
            fatal("NO SE PUDO LEER EL CONST SEGMENT");
    }

    fclose(arch);
}

void calculateSegments(type_machine *machine, int paraMainSizes, char route[]) {
    int8_t res = 0;
    uint16_t cs_size, ds_size, es_size, ss_size, constSeg, entry_point;

    readHeader(route, &cs_size, &res, &ds_size, &es_size, &ss_size, &constSeg, &entry_point);
    if (!res)
        fatal("CABECERA INVALIDA: EL ARCHIVO NO ES UN PROGRAMA VMX26 VERSION 1");
    if (cs_size > machine->memory.n_mem)
        fatal("espacio insuficiente");

    initTableSeg(machine->segments);

    int i = 0;
    if (paraMainSizes != 0) {
        addSegment(machine->segments, i, paraMainSizes);
        initRegSegment(i, PS, machine->registers);
        i++;
    }
    if (constSeg != 0) {
        addSegment(machine->segments, i, constSeg);
        initRegSegment(i, KS, machine->registers);
        i++;
    }
    // se añade el cs
    int physbasecs = (i == 0) ? 0 : highest(machine->segments[i - 1]) + lowest(machine->segments[i - 1]);
    addSegment(machine->segments, i, cs_size);
    initRegSegment(i, CS, machine->registers);
    if (entry_point >= cs_size)
        fatal("ENTRY POINT FUERA DEL CODE SEGMENT");
    // la ejecucion arranca en el entry point, no necesariamente en el offset 0
    machine->registers[IP].value = machine->registers[CS].value | entry_point;
    i++;
    // se añade el ds
    addSegment(machine->segments, i, ds_size); // data segment
    initRegSegment(i, DS, machine->registers);
    i++;

    if (es_size != 0) {
        addSegment(machine->segments, i, es_size);
        initRegSegment(i, ES, machine->registers);
        i++;
    }
    if (ss_size != 0) {
        addSegment(machine->segments, i, ss_size);
        initRegSegment(i, SS, machine->registers);
        i++;
    }
}