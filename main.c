#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.c"
#include "headers/operators.h"


void createTableSeg (uint32_t TBS[], uint16_t cs_size){
    TBS[0] = 0;
    TBS[0] |= (cs_size & 0xFFFF);
    TBS[1] = 0;
    TBS[1] |= cs_size;
    TBS[1] = TBS[1]<<16;
    TBS[1] = TBS[1] | ( N_MEM - cs_size);

     for(int i=2;i<N_SEG;i++){
        TBS[i] = 0xFFFF;
        TBS[i] = TBS[i] << 16;
        TBS[i] = TBS[i] | 0xFFFF;
    }
   
    /*for(int i=0;i<8;i++)
        printf("0x%08X\n", TBS[i]);*/

    
    
}


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

void uploadMem(char *argv[], int8_t memory[],uint16_t *cs_size){
    FILE *arch = fopen(*(argv + 1), "rb");  //abre el archivo indicado por parametro

    uint16_t code_size; //guardamos el tamaño del code en una var de 2bytes
    int8_t res = 0;     //guarda si es posible ejecutar el programa .vmx

    readHeader(*(argv + 1), &code_size, &res);
    *cs_size=code_size;

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
        printf( "\n");
    }
}

int main(int argc, char *argv[])
{
    type_machine machine;

    operatorASM operators[N_OP] = OPERATORS;

    printf("%s\n",operators[0].name);   //muestra el operador de indice 0
    uint16_t  cs_size;
    uploadMem(argv, machine.memory,&cs_size);
    createTableSeg(machine.segments,cs_size);  //se crea la tabla de segmentos


    return 0;
}
