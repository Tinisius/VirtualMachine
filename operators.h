#include "operators.c"
#define OPERATORS {{"MOV", 0x10, MOV},{"ADD", 0x11, ADD}, {"MUL", 0x13, MUL}}

typedef struct
{
    char *name;
    int16_t code;
    void (*operation)(int32_t, int32_t, type_machine);
} operatorASM;


void MOV(int32_t, int32_t, type_machine);

void ADD(int32_t, int32_t, type_machine);

void MUL(int32_t, int32_t, type_machine);
