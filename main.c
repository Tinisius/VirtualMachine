#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.c"
#include "headers/operators.h"

uint32_t obtenerdireccionFisica(type_machine m,int32_t dirlogica){
    uint16_t indiceseg = highest(dirlogica);
    if (indiceseg < N_SEG){
        uint16_t offset = lowest(dirlogica);
        uint16_t base = highest(m.segments[indiceseg]);
        return  base + offset;
    }
    else{
        printf("te pasaste de segmentos\n");
        exit(-1);

    }


}


int corresponds( type_machine *m){

    if (m->registers[IP].value < 0)
        return 0;
    else{
        uint32_t table = m->segments[highest(m->registers[CS].value)];
        uint16_t base= highest(table);
        uint16_t tam = lowest(table);
        int16_t ipDireccionFisica = obtenerdireccionFisica(*m,m->registers[IP].value);
        ipDireccionFisica-=base;
        printf("%0x--\n",ipDireccionFisica >=0 && ipDireccionFisica <tam);
        return ipDireccionFisica >=0 && ipDireccionFisica <tam;
    }

}

void initRegs( reg regs[]){
    regs[IP].value = 0;
    regs[CS].value = 0;
    regs[DS].value = 0x00010000;

}

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
    //muestra la tabla de descriptores de segmentos

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
    initRegs(machine.registers);

    while( corresponds(&machine) ){ //analiza si corresponde leer/seguir leyendo las instrucciones
        
        uint32_t indiceFisico = obtenerdireccionFisica(machine,machine.registers[IP].value);
        uint8_t instruccion = machine.memory[indiceFisico];
        
        uint8_t tipeB = (instruccion >> 6) & 0x03;
        uint8_t tipeA = (instruccion >> 4) & 0x03;
        uint8_t opC = instruccion & 0x1F; 

        machine.registers[OPC].value = opC;

        //como ya lei un byte el indice debe incrementarse para leer el operando B
        indiceFisico++;
        int32_t valorB = leerValor(machine.memory,tipeB,&indiceFisico);
        int32_t valorA = leerValor(machine.memory,tipeA,&indiceFisico);

        if (tipeA == 0)
        {
            tipeA = tipeB;
            valorA = valorB;
            tipeB = valorB = 0;
        }
        machine.registers[OP2].value = ((int32_t)tipeB << 24) | (valorB & 0x00FFFFFF);
        machine.registers[OP1].value = ((int32_t)tipeA << 24) | (valorA & 0x00FFFFFF);
        machine.registers[IP].value+=1+tipeA+tipeB;

        printf("%0x %0x_opa %0x_opb\n",machine.registers[IP].value, machine.registers[OP1].value,machine.registers[OP2].value );




    }


    return 0;
}
