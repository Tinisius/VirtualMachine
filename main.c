#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> //libreria para estandarizar el tamaño en bytes de los tipos
#include "utils.c"

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
    char *name;
    int16_t code;
    void (*operation)(int32_t, int32_t);
} operatorASM;

void readHeader(char route[], uint16_t *code_size, int8_t *res)
{
    uint8_t line[N_HEADER];
    FILE *arch = fopen(route, "rb");

    if (fread(line, 1, N_HEADER, arch) == N_HEADER)
    {
        // Bytes 0-4: identificador "VMX26"
        // Byte 5: version
        // Bytes 6-7: tamano codigo
        *code_size = ((uint16_t)line[6] << 8) | line[7];
        // uso memcmp porque line no es una cadena terminada en \0. comparo byte a byte contra ID
        *res = (memcmp(line, ID, 5) == 0) && (line[5] == VERSION) && ((*code_size) <= N_MEM - 1);

        // TEST: mostrar lectura
        printf("IDENTIFICADOR: \"%.5s\"\n", line);
        printf("VERSION: %d\n", line[5]);
        printf("TAMANO EN BYTES: %u\n", *code_size);
    }
    else
        *res = 0;
    fclose(arch);
}

void uploadMem(char *argv[]){
    FILE *arch = fopen(*(argv + 1), "rb");  //abre el archivo indicado por parametro
    int8_t memory[N_MEM];
    // reg registers[N_REG];
    // int32_t segments[N_SEG];
    // operatorASM operations[N_OP];

    uint16_t code_size; //guardamos el tamaño del code en una var de 2bytes
    int8_t res = 0;     //guarda si es posible ejecutar el programa .vmx

    readHeader(*(argv + 1), &code_size, &res);

    if (res)
    {
        fseek(arch, N_HEADER, SEEK_SET);

        // aca faltaria inicializar CS
        fread(memory, 1, code_size, arch);  //guarda en memoria todo el code segment
        fclose(arch);

        for (int i = 0; i < code_size; i++)
        {
            printf("%02x \t", memory[i]);   //muestra toda la memoria
        }
    }
}

int main(int argc, char *argv[])
{

    uploadMem(argv);

    return 0;
}
