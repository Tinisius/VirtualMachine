#include "../headers/utils.h"
#include "../headers/constants.h"
#include "../headers/operators.h"
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

    if (physicAdr < base || physicAdr >= base + size || physicAdr >= N_MEM)
        fatal("FALLO DE SEGMENTO");

    return 1;
}

int memWrite(int32_t logicAdr, int16_t size, type_machine *m, int32_t value, int *error) {
    int32_t physicalAdr = obtainPhysicAdr(m, logicAdr);

    m->registers[LAR].value = logicAdr;
    m->registers[MAR].value = (size << 16) | (physicalAdr & 0xFFFF);
    m->registers[MBR].value = value;

    for (int i = 0; i < size; i++) {
        if (inSegment(physicalAdr + i, logicAdr, m)) {
            m->memory[physicalAdr + i] = ((uint32_t)value >> ((size - i - 1) * 8)) & 0xFF;
        } else {
            *error = 1;
            return 1;
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
            data |= (uint32_t)m->memory[physicalAdr + i] << ((size - i - 1) * 8);
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
        if (type_op == 3) {
            memRead(getOPLogicAdress(op, m), 4, m->registers[DS].value, m);
            value = m->registers[MBR].value;

        } else {
            uint8_t reg = op & 0x1F;
            value = m->registers[reg].value;
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

void setOPValue(uint32_t OP, type_machine *m, int32_t newValue) {
    int8_t typeA = getOpType(OP);
    int error = 0;

    if (typeA == 1) { // registro
        m->registers[OP & 0x1F].value = newValue;
    } else {
        if (typeA == 3) { // op memoria
            // estaba en 16?
            uint32_t logic = getOPLogicAdress(OP, m);
            m->registers[MBR].value = newValue;
            m->registers[LAR].value = logic;
            memWrite(logic, 4, m, newValue, &error);
            if (error)
                fatal("ERROR DE MEMORIA");
        } else
            fatal("NO SE PUEDE ESCRIBIR EN UN OPERANDO INMEDIATO");
    }
}

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
    printf("\n");
    for (int i = 0; i < N_REG; i++) {
        printf("%s \t", machine.registers[i].name, machine.registers[i].value);
        printBin(machine.registers[i].value, 4);
        printf("\n");
    }
    printf("\n");
    // //ESCRIBE MEMORIA
    for (int i = 0; i < 128; i++) {
        printf("%d  ", i);
        printBin(machine.memory[i], 1);
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
    while (isspace((unsigned char)*p))  //valida si sos un mono
        p++;
    int negative = *p == '-';
    if (*p == '-' || *p == '+')
        p++;
    if (prefix && p[0] == '0' && tolower((unsigned char)p[1]) == prefix)
        p += 2;

    char *end;
    errno = 0;
    long long value = strtoll(p, &end, base);   //valor, castea el string a el numero entero en cuestion, ej: "200   " = 200
    while (isspace((unsigned char)*end))
        end++;
    // tiene que haber al menos un digito, sin otro signo, y nada despues del numero
    if (*p == '-' || *p == '+' || *end != '\0')
        fatal("ENTRADA INVALIDA");

    return (int32_t)(uint32_t)(negative ? -value : value);
}