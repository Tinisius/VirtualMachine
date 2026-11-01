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

void readHeader(char route[], uint16_t *code_size, int8_t *res, uint16_t *ds_size, uint16_t *es_size, uint16_t *ss_size, uint16_t *const_size, uint16_t *entry_point, uint8_t *version) {
    uint8_t line[N_HEADER_V2]; // N_HEADER AHORA VALE 18 BYTES
    FILE *arch = fopen(route, "rb");

    if (arch == NULL)
        fatal("NO SE PUDO ABRIR EL ARCHIVO");
    if (fread(line, 1, N_HEADER_V1, arch) == N_HEADER_V1) {
        // Bytes 0-4: identificador "VMX26"
        // Byte 5: version
        // Bytes 6-7: tamaño codigo
        *version = line[5];
        *code_size = ((uint16_t)line[6] << 8) | line[7];
        // uso memcmp porque line no es una cadena terminada en \0. comparo
        // byte a byte contra ID
        *res = (memcmp(line, ID, 5) == 0) && (*version == 1 || *version == 2);
    } else
        *res = 0;

    if (*version == 2) {
        if (fread(line + N_HEADER_V1, 1, N_HEADER_V2 - N_HEADER_V1, arch) == N_HEADER_V2 - N_HEADER_V1) {

            *ds_size = ((uint16_t)line[8] << 8) | line[9];
            *es_size = ((uint16_t)line[10] << 8) | line[11];
            *ss_size = ((uint16_t)line[12] << 8) | line[13];
            *const_size = ((uint16_t)line[14] << 8) | line[15];
            // Bytes 16-17: entry point (offset dentro del code segment)
            *entry_point = ((uint16_t)line[16] << 8) | line[17];
        }
    }
    fclose(arch);
}

void uploadMem(type_machine *mv, type_entry_params params, uint8_t version) {
    uint8_t offset = version == 1 ? N_HEADER_V1 : N_HEADER_V2;
    FILE *arch = fopen(params.vmx_file, "rb");
    if (arch == NULL)
        fatal("NO SE PUDO ABRIR EL ARCHIVO");

    fseek(arch, offset, SEEK_SET);

    // guarda parametros en el PS (solo si hay parametros opcionales)
    if (params.n_op_params > 0) {
        int32_t ps = mv->registers[PS].value;

        // calcula el tamaño a ocupar en el PS
        uint32_t strings_size = strlen(params.vmx_file) + 1;
        for (int i = 0; i < params.n_op_params; i++)
            strings_size += strlen(params.optional_params[i]) + 1;

        int32_t argv_adr = ps + strings_size;
       PUSH(argv_adr,argv_adr,mv);

        uint32_t offset = 0;
        for (int k = 0; k <= params.n_op_params; k++) {
            char *str = (k == 0) ? params.vmx_file : params.optional_params[k - 1];
            size_t len = strlen(str);

            memWrite(argv_adr + 4 * k, 4, mv, ps + offset);
            for (size_t j = 0; j <= len; j++)             // escribe el parametro en el PS
                memWrite(ps + offset + j, 1, mv, str[j]); // el j == len escribe el \0
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

void calculateSegments(type_machine *machine, int paraMainSizes, char route[], uint8_t *version) {
    int8_t res = 0;
    uint16_t cs_size, ds_size, es_size, ss_size, constSeg, entry_point;

    readHeader(route, &cs_size, &res, &ds_size, &es_size, &ss_size, &constSeg, &entry_point, version);
    if (!res)
        fatal("CABECERA INVALIDA: EL ARCHIVO NO ES UN PROGRAMA VMX26 VERSION VALIDA");
    if (cs_size == 0)
        fatal("CODE SEGMENT VACIO");
    if (entry_point >= cs_size)
        fatal("ENTRY POINT FUERA DEL CODE SEGMENT");

    // orden fisico = orden en la tabla
    const int regOf[6]     = {PS, KS, CS, DS, ES, SS};
    const uint32_t size[6] = {(uint32_t)paraMainSizes, constSeg, cs_size, ds_size, es_size, ss_size};

    // 1) la suma de TODOS los segmentos tiene que entrar en la memoria (en 32 bits, sin desborde)
    uint32_t total = 0;
    for (int k = 0; k < 6; k++) {
        if (size[k] > 0xFFFF)
            fatal("SEGMENTO DEMASIADO GRANDE (MAXIMO 65535)");
        total += size[k];
    }
    if (total > (uint32_t)machine->memory.n_mem)
        fatal("MEMORIA INSUFICIENTE: LOS SEGMENTOS NO ENTRAN EN LA MEMORIA PRINCIPAL");

    initTableSeg(machine->segments);

    // 2) todos los registros de segmento arrancan ausentes (-1)
    for (int k = 0; k < 6; k++)
        machine->registers[regOf[k]].value = 0xFFFFFFFF;

    // 3) entradas solo para los segmentos con tamaño > 0
    uint32_t base = 0;
    int i = 0;
    for (int k = 0; k < 6; k++) {
        if (size[k] == 0)
            continue;
        if (base > 0xFFFF)
            fatal("BASE DE SEGMENTO NO REPRESENTABLE EN 16 BITS");

        addSegment(machine->segments, i, size[k]);
        initRegSegment(i, regOf[k], machine->registers);

        if (regOf[k] == CS)   // la ejecucion arranca en el entry point
            machine->registers[IP].value = machine->registers[CS].value | entry_point;
        if (regOf[k] == SS)   // pila vacia: SP = SS + tamaño (justo afuera del segmento)
            machine->registers[SP].value = ((uint32_t)i << 16) | (ss_size & 0xFFFF);

        base += size[k];
        i++;
    }
}
// carga la memoria, los registros y la tabla de segmentos tal como estan en la imagen
void readVMI(type_machine *mv, char route[]) {
    uint8_t header[8], bytes[4];
    FILE *arch = fopen(route, "rb");

    if (arch == NULL)
        fatal("NO SE PUDO ABRIR EL ARCHIVO");
    if (fread(header, 1, 8, arch) != 8 || memcmp(header, "VMI26", 5) != 0 || header[5] != 1)
        fatal("CABECERA INVALIDA: EL ARCHIVO NO ES UNA IMAGEN VMI26 VERSION VALIDA");

    // bytes 6-7: tamaño de la memoria en KiB
    mv->memory.n_mem = (((uint32_t)header[6] << 8) | header[7]) * 1024;
    mv->memory.mem_values = calloc(mv->memory.n_mem, sizeof(*mv->memory.mem_values));
    if (mv->memory.mem_values == NULL)
        fatal("NO SE PUDO RESERVAR LA MEMORIA");

    for (int i = 0; i < N_REG; i++) {
        if (fread(bytes, 1, 4, arch) != 4)
            fatal("NO SE PUDIERON LEER LOS REGISTROS");
        mv->registers[i].value = ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) | ((uint32_t)bytes[2] << 8) | bytes[3];
    }

    for (int i = 0; i < N_SEG; i++) {
        if (fread(bytes, 1, 4, arch) != 4)
            fatal("NO SE PUDO LEER LA TABLA DE SEGMENTOS");
        mv->segments[i] = ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) | ((uint32_t)bytes[2] << 8) | bytes[3];
    }

    if (fread(mv->memory.mem_values, 1, mv->memory.n_mem, arch) != mv->memory.n_mem)
        fatal("NO SE PUDO LEER LA MEMORIA");

    fclose(arch);
}
