#include <stdint.h> //libreria para estandarizar el tamaño en bytes de los tipos

#define N_HEADER 8
#define ID "VMX26"
#define VERSION 1

#define N_MEM 16384
#define N_SEG 8
#define N_REG 32
#define N_OP 32

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

typedef struct
{
    char *name;
    int32_t value;
} reg;


typedef struct
{
    int8_t memory[N_MEM];
    reg registers[N_REG];
    int32_t segments[N_SEG];
    
}type_machine;
