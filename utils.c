#include <stdint.h>

int lsh(int number, int shift){
    //shifting func
    return number << shift;
}


uint16_t highest(uint32_t x){
    return (x >> 16) & 0xFFFF;
    

}

uint16_t lowest(uint32_t x){
    return x & 0xFFFF;

}

int32_t leerValor(int8_t mem[], uint8_t cantBytesOperando,uint32_t *indiceFisico){
    //EL VALOR PUEDE SER NEGATIVO BOLUDO
    if (cantBytesOperando == 0) 
        return 0;

    int32_t v;
    v =  mem[*indiceFisico];
    v <<= 24;
    v >>= 24; 
    cantBytesOperando--
    ;
    for (int i = 0;  i<cantBytesOperando; i++){
        v<<=8;
        v |= (uint8_t)mem[++(*indiceFisico)]; //casteo a sin signo para evitar arrastrar basura negativa

    }
    (*indiceFisico)++;
    return v;

}