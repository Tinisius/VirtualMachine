// proteccion estandar
// evita definirse mas de una vez
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdint.h> //libreria para estandarizar el tamaño en bytes de los tipos

#define MEM_DEFAULT 16384

#define N_HEADER 18
#define ID "VMX26"
#define VERSION 1

#define N_SEG 8
#define N_REG 32
#define N_OP 32

// registros
#define IP 0
#define OPC 1
#define OP1 2
#define OP2 3
#define LAR 4
#define MAR 5
#define MBR 6
#define EAX 10
#define EBX 11
#define ECX 12
#define EDX 13
#define EEX 14
#define EFX 15
#define AC 16
#define CC 17
#define CS 26
#define DS 27
#define ES 28
#define SS 29
#define KS 30
#define PS 31

#define REG_NAMES                                                                                                                                                                                      \
    {"IP", "OPC", "OP1", "OP2", "LAR", "MAR", "MBR", "-", "-", "-", "EAX", "EBX", "ECX", "EDX", "EEX", "EFX", "AC", "CC", "-", "-", "-", "-", "-", "-", "-", "-", "CS", "DS", "-", "-", "-", "-"}

typedef struct {
    char *name;
    int32_t value;
} reg;

typedef struct {
    uint8_t *mem_values;
    uint32_t n_mem;
} type_memory;

typedef struct {
    type_memory memory;
    reg registers[N_REG];
    uint32_t segments[N_SEG]; // base (2 bytes altos) y tamaño (2 bytes bajos), sin signo
} type_machine;

typedef struct {
    char *vmx_file;
    char *vmi_file;
    uint32_t n_mem;
    uint8_t disassembler;
    char **optional_params;
    uint8_t n_op_params;
} type_entry_params;

// proteccion estandar
#endif
