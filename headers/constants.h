#include <stdint.h> //libreria para estandarizar el tamaño en bytes de los tipos

#define N_HEADER 8
#define ID "VMX26"
#define VERSION 1

#define N_MEM 16384
#define N_SEG 8
#define N_REG 32
#define N_OP 32



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
