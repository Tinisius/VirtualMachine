#include "../headers/utils.h"
#include "../headers/constants.h"
#include "../headers/operators.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint16_t highest(uint32_t x) { return (x >> 16) & 0xFFFF; }

uint16_t lowest(uint32_t x) { return x & 0xFFFF; }

// informa el error y aborta la ejecucion del proceso
void fatal(const char *msg) {
    printf("\nERROR: %s\n", msg);
    exit(-1);
}

// sigue la ejecucion mientras IP apunte dentro del segmento de codigo
// (STOP deja IP en -1, cuyo codigo de segmento 0xFFFF no coincide con el de CS)
int corresponds(type_machine *m) {
    uint32_t ip = m->registers[IP].value;
    uint16_t segmIndex = highest(ip);
    return segmIndex == highest(m->registers[CS].value) && lowest(ip) < lowest(m->segments[segmIndex]);
}

uint16_t obtainPhysicAdr(type_machine *m, int32_t logicAdr) {
    uint16_t segmIndex = highest(logicAdr);
    if (segmIndex >= N_SEG)
        fatal("FALLO DE SEGMENTO");

    uint16_t offset = lowest(logicAdr);
    uint16_t base = highest(m->segments[segmIndex]);
    return base + offset;
}

// valida que physicAdr caiga dentro del segmento de la direccion logica logicAdr
// (su parte alta es el indice en la tabla de segmentos); si no, aborta
int inSegment(int32_t physicAdr, int32_t logicAdr, type_machine *m) {
    uint16_t segmIndex = highest(logicAdr);
    if (segmIndex >= N_SEG || m->segments[segmIndex] == 0xFFFFFFFF)
        fatal("FALLO DE SEGMENTO");

    uint16_t base = highest(m->segments[segmIndex]);
    uint16_t size = lowest(m->segments[segmIndex]);

    if (physicAdr < base || physicAdr >= base + size || physicAdr >= m->memory.n_mem)
        fatal("FALLO DE SEGMENTO");

    return 1;
}

int memWrite(int32_t logicAdr, int16_t size, type_machine *m, int32_t value) {
    int32_t physicalAdr = obtainPhysicAdr(m, logicAdr);

    m->registers[LAR].value = logicAdr;
    m->registers[MAR].value = (size << 16) | (physicalAdr & 0xFFFF);
    m->registers[MBR].value = value;

    for (int i = 0; i < size; i++) {
        if (inSegment(physicalAdr + i, logicAdr, m)) {
            printf("in segment, direccion logica: %d \n", logicAdr);
            m->memory.mem_values[physicalAdr + i] = ((uint32_t)value >> ((size - i - 1) * 8)) & 0xFF;
        } else {
            fatal("FALLO DE SEGMENTO");
        }
    }
    return 0;
}

// lee size bytes a partir de logicAdr y devuelve el valor leido.
// segment == CS indica que se esta leyendo una instruccion: no se valida y no se
// modifican LAR, MAR ni MBR. Los demas accesos se validan contra el segmento de la
// direccion logica y dejan cargados LAR, MAR y MBR
uint32_t memRead(int32_t logicAdr, int16_t size, int32_t segment, type_machine *m) {
    uint16_t physicalAdr = obtainPhysicAdr(m, logicAdr);
    int isInstruction = segment == m->registers[CS].value;

    if (!isInstruction) {
        m->registers[LAR].value = logicAdr;
        m->registers[MAR].value = (size << 16) | (physicalAdr & 0xFFFF);
    }

    uint32_t data = 0;
    for (int i = 0; i < size; i++) {
        if (isInstruction || inSegment(physicalAdr + i, logicAdr, m))
            data |= (uint32_t)m->memory.mem_values[physicalAdr + i] << ((size - i - 1) * 8);
    }

    if (!isInstruction)
        m->registers[MBR].value = data;
    return data;
}

void printBin(int32_t value, int16_t size) {
    for (int i = size - 1; i >= 0; i--) { // cada byte  (1 - size)  (validar endianes)
        for (int j = 7; j >= 0; j--)      // escribe el byte (1 - 8)
            printf("%0x", (value >> (j + i * 8)) & 0b1);
        printf(" ");
    }
}
int negativeCC(uint32_t cc) { return (cc >> 31) & 0x01; }

int zeroCC(uint32_t cc) { return ((cc << 1) >> 31) & 0x01; }

int carryCC(uint32_t cc) { return ((cc << 2) >> 31) & 0x01; }

int overflowCC(uint32_t cc) { return ((cc << 3) >> 31) & 0x01; }

int32_t getOPValue(uint32_t op, type_machine *m) {
    uint8_t type_op = getOpType(op);
    // inmediato: 2 bytes en complemento a 2, hay que extender el signo a 32
    int32_t value = (int16_t)(op & 0xFFFF);

    if (type_op != 2) {
        if (type_op == 3) { // operando de memoria
            value = m->registers[MBR].value;
            uint8_t mem_size = (op >> 5) & 0x07; // modificador de tamaño
            memRead(getOPLogicAdress(op, m), mem_size, m->registers[DS].value, m);

        } else { // operando de registro
            uint8_t reg = op & 0x1F;
            value = m->registers[reg].value;

            uint8_t reg_size = (op >> 5) & 0x03; // modificador de tamaño
            switch (reg_size) {
            case 1: // AL (4to byte)
                value = value & 0xFF;
                break;
            case 2: // AH (3er byte)
                value = (value >> 8) & 0xFF;
                break;
            case 3: // AX (3er y 4to byte)
                value = value & 0xFFFF;
                break;
            }
        }
    }

    return value;
}

uint8_t getOpType(uint32_t op) { return (uint8_t)((op >> 24) & 0x00000003); }

uint32_t getOPLogicAdress(uint32_t op, type_machine *m) {
    uint32_t adress = m->registers[op & 0x1F].value; // EJ: DS = 0001 0000 0000 0000
    if (getOpType(op) == 3) {                        // el DS es  00 01 00 00
        adress += (int16_t)((op >> 8) & 0xFFFF);     // offset con signo
    }

    return adress;
}

void setOPValue(uint32_t op, type_machine *m, int32_t newValue) {
    int8_t typeA = getOpType(op);

    if (typeA == 1) {                        // operando de registro
        uint8_t reg_size = (op >> 5) & 0x03; // modificador de tamaño
        switch (reg_size) {
        case 1: // AL (4to byte)
            newValue = newValue & 0xFF;
            break;
        case 2: // AH (3er byte)
            newValue = (newValue >> 8) & 0xFF;
            break;
        case 3: // AX (3er y 4to byte)
            newValue = newValue & 0xFFFF;
            break;
        }
        m->registers[op & 0x1F].value = newValue;
    } else {
        if (typeA == 3) { // operando de memoria
            uint32_t logicAdr = getOPLogicAdress(op, m);
            m->registers[MBR].value = newValue;
            m->registers[LAR].value = logicAdr;
            uint8_t mem_size = (op >> 5) & 0x07; // modificador de tamaño
            memWrite(logicAdr, mem_size, m, newValue);
        } else
            fatal("NO   SE PUEDE ESCRIBIR EN UN OPERANDO INMEDIATO");
    }
}

// actualiza N, Z, C y V del registro CC; result viene sin truncar y cc_mode
// indica de que operacion salio (0 logica, 1 suma, 2 resta, 3 resultado ancho)
// N, Z, C y V son los bits 31, 30, 29 y 28 del CC (el resto es reservado).
//
// result es el resultado de la operacion SIN truncar a 32 bits, porque el modo 3
// necesita ver los bits que no entran en la palabra del procesador.
//
// cc_mode dice de que operacion viene el resultado, porque C y V se calculan
// de manera distinta en cada caso:
//   0 = logica (MOV, AND, OR, XOR, NOT, SWAP): no puede desbordar, C y V van en 0
//   1 = suma (ADD)
//   2 = resta (SUB, CMP): se hace como A + (-B)
//   3 = resultado ancho (MUL, DIV, SHL, SHR, SAR): C y V salen de comparar el
//       resultado real contra los 32 bits disponibles
void uploadcc(int32_t valA, int32_t valB, int64_t result, type_machine *m, int cc_mode) {
    int32_t truncated = (int32_t)result; // lo que realmente queda en el destino
    uint32_t cc = 0;
    if (truncated < 0) // N: el resultado es negativo
        cc |= 0x80000000;

    if (truncated == 0) // Z: el resultado es cero
        cc |= 0x40000000;

    switch (cc_mode) {
    case 1: // suma
        // C: la suma sin signo dio la vuelta, el resultado quedo mas chico que A
        if ((uint32_t)truncated < (uint32_t)valA)
            cc |= 0x20000000;

        // V: los dos operandos tienen igual signo y el resultado sale con el contrario
        if (((valA ^ truncated) & (valB ^ truncated)) < 0)
            cc |= 0x10000000;
        break;

    case 2: // resta, que es la suma A + (-B)
        // C: acarreo de salida de esa suma, o sea que la resta no pidio prestado
        if ((uint32_t)valA >= (uint32_t)valB)
            cc |= 0x20000000;

        // V: los operandos tienen distinto signo y el resultado sale con el signo de B
        // EQUIVALE A: signo(A) != signo(B) && signo(resultado) != signo(A)
        // 10 - (-5) = 15 <- SIGNO DE A / -10 - 5 = -15 <- SIGNO DE A
        // SI NO SE CUMPLE LO ANTERIOR HAY OVERFLOW => SE ROMPIO
        if (((valA ^ valB) & (valA ^ truncated)) < 0)
            cc |= 0x10000000;
        break;

    case 3: // el resultado real puede no entrar en 32 bits
        // C: excede los 32 bits del procesador, ni con signo ni sin el
        // if(result > 0xFFFFFFFFLL || result < -0x80000000LL)
        if ((result >> 32) & 1)
            cc |= 0x20000000;

        // V: lo que quedo truncado no es el resultado real
        if (result != truncated)
            cc |= 0x10000000;
        break;
    }

    m->registers[CC].value = cc;
}

int32_t arShiftRight(int32_t value, int32_t shift) {
    if ((value >> 31) & 0b1) { // si es negativo
        for (int i = 0; i < shift; i++) {
            value = (value >> 1) | (0b1 << 31);
        }
        return value;
    } else
        return value >> shift;
}

// separa con un espacio cada formato escrito, salvo el primero
static void separator(int *first) {
    if (!*first)
        putchar(' ');
    *first = 0;
}

// escribe el valor de una celda de size bytes en cada modo activo en mode (EAX),
// en el orden del enunciado: binario, hexadecimal, octal, caracteres y decimal
void printFormat(int32_t value, int32_t mode, int16_t size) {
    uint32_t u = (uint32_t)value;
    int first = 1;

    if (mode & 0x10) { // binario sin ceros a la izquierda
        separator(&first);
        int msb = 31;
        while (msb > 0 && !((u >> msb) & 1))
            msb--;
        printf("0b");
        for (int b = msb; b >= 0; b--)
            putchar('0' + ((u >> b) & 1));
    }
    if (mode & 0x08) {
        separator(&first);
        printf("0x%X", u);
    }
    if (mode & 0x04) {
        separator(&first);
        printf("0o%o", u);
    }
    if (mode & 0x02) { // un caracter por cada byte de la celda, '.' si no es imprimible
        separator(&first);
        for (int i = size - 1; i >= 0; i--) {
            uint8_t c = (u >> (i * 8)) & 0xFF;
            putchar(c >= 32 && c <= 126 ? c : '.');
        }
    }
    if (mode & 0x01) {
        separator(&first);
        printf("%d", value);
    }
}

void logHeader(char *id, char v, uint16_t code_size) {
    printf("IDENTIFICADOR: \"%.5s\"\n", id);
    printf("VERSION: %d\n", v);
    printf("TAMANO EN BYTES: %u\n", code_size);
}

void logMachine(type_machine machine) {
    // ESCRIBE REGISTROS
    printf("REGISTROS: \n");
    for (int i = 0; i < N_REG; i++) {
        printf("%s \t", machine.registers[i].name, machine.registers[i].value);
        printBin(machine.registers[i].value, 4);
        printf("\n");
    }
    printf("SEGMENTOS: \n");
    // //ESCRIBE SEGMENTOS
    for (int i = 0; i < N_SEG; i++) {
        printf("%d  ", i);
        printBin(machine.segments[i], 4);
        printf("\n");
    }
    printf("MEMORIA: \n");
    // ESCRIBE MEMORIA
    for (int i = 0; i < 128; i++) {
        printf("%d  ", i);
        printBin(machine.memory.mem_values[i], 1);
        printf("\n");
    }

    printf("\n");
}

int32_t readSysValue(int32_t mode) {
    // Si hay varios bits activos se usa el primero en ese orden. Los numeros aceptan
    // signo y, opcionalmente, el prefijo de su base (0o, 0x, 0b)
    char line[128];
    if (fgets(line, sizeof line, stdin) == NULL)
        fatal("ENTRADA INVALIDA");
    line[strcspn(line, "\r\n")] = '\0';

    if (mode & 0x02) { // caracter: se guarda su codigo ASCII
        return (uint8_t)line[0];
    }

    int base = 10; // decimal (0x01)
    char prefix = 0;
    if (!(mode & 0x01)) {
        if (mode & 0x04)
            base = 8, prefix = 'o';
        else if (mode & 0x08)
            base = 16, prefix = 'x';
        else if (mode & 0x10)
            base = 2, prefix = 'b';
        else
            fatal("MODO DE LECTURA INVALIDO");
    }

    char *p = line;
    while (isspace((unsigned char)*p)) // valida si sos un mono
        p++;
    int negative = *p == '-';
    if (*p == '-' || *p == '+')
        p++;
    if (prefix && p[0] == '0' && tolower((unsigned char)p[1]) == prefix)
        p += 2;

    char *end;
    errno = 0;
    long long value = strtoll(p, &end, base); // valor, castea el string a el numero entero en cuestion, ej: "200   " = 200
    while (isspace((unsigned char)*end))
        end++;
    // tiene que haber al menos un digito, sin otro signo, y nada despues del numero
    if (*p == '-' || *p == '+' || *end != '\0')
        fatal("ENTRADA INVALIDA");

    return (int32_t)(uint32_t)(negative ? -value : value);
}

// Lee una linea en dest (capacidad cap). Devuelve el largo, o -1 si fallo.
uint16_t readSysString(char *str, uint16_t maxSize) {
    if (maxSize == 0 || fgets(str, (int)maxSize + 1, stdin) == NULL) // lee la linea, guarad en str
        fatal("ENTRADA INVALIDA");

    uint16_t len = strcspn(str, "\r\n"); // el largo del string, "hola\0" => 4

    if (str[len] == '\0' && len == maxSize - 1) { // si termina en \0 (deberia terminar en \n) y el buffer se lleno
        int c;
        while ((c = getchar()) != '\n' && c != EOF) // la linea no entro completa: descartar el resto hasta el salto
            ;
    }
    str[len] = '\0'; // agrega el \0 al final (remplaza el \n)
    return len;
}

bool endsWith(char *s, char *ext) {
    size_t ls = strlen(s), le = strlen(ext);
    return ls > le && strcmp(s + ls - le, ext) == 0;
}

int sizeObtain(int32_t OP) {
    uint8_t type = OP >> 24 & 0XFF;
    int size;

    switch (type) {
    case 1: { // registro
        uint8_t sector = (OP >> 5) & 0x03;
        if (sector == 0b00)
            size = 4;
        else if (sector == 0b11)
            size = 2;
        else
            size = 1;
        break;
    }
    case 2: { // inmediato
        size = 2;
        break;
    }
    case 3: { // memoria
        uint8_t mod = (OP >> 5) & 0x07;
        if (mod == 0b001)
            size = 1; // b[..]
        else if (mod == 0b010)
            size = 2; // W[..]
        else
            size = 4; // [..] o l[..]
        break;
    }
    }
    return size;
}

void genVMI(type_machine mv) {

    FILE *vmi_file = fopen(mv.vmi_file, "wb");

    if (vmi_file == NULL)
        fatal("NO SE PUEDE CREAR EL ARCHIVO VMI");

    uint16_t mem_kib = mv.memory.n_mem / 1024;
    uint8_t header[8] = {'V', 'M', 'I', '2', '6', 1, (mem_kib >> 8) & 0xFF, mem_kib & 0xFF};
    fwrite(header, 1, sizeof(header), vmi_file);

    for (int i = 0; i < N_REG; i++) {
        uint32_t value = (uint32_t)mv.registers[i].value;
        uint8_t bytes[4] = {(value >> 24) & 0xFF, (value >> 16) & 0xFF, (value >> 8) & 0xFF, value & 0xFF};
        fwrite(bytes, 1, 4, vmi_file);
    }

    for (int i = 0; i < N_SEG; i++) {
        uint32_t value = mv.segments[i];
        uint8_t bytes[4] = {(value >> 24) & 0xFF, (value >> 16) & 0xFF, (value >> 8) & 0xFF, value & 0xFF};
        fwrite(bytes, 1, 4, vmi_file);
    }

    fwrite(mv.memory.mem_values, 1, mv.memory.n_mem, vmi_file);

    fclose(vmi_file);
}