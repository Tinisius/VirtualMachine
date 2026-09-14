#include "../operators.c"
#define OPERATORS                                                              \
    {                                                                          \
        {"SYS", 0x00, SYS}, {"JMP", 0x01, JMP}, {"JP", 0x02, JP},              \
            {"JN", 0x03, JN}, {"JZ", 0x04, JZ}, {"JC", 0x05, JC},              \
            {"JV", 0x06, JV}, {"JNP", 0x07, JNP}, {"JNN", 0x08, JNN},          \
            {"JNZ", 0x09, JNZ}, {"NOT", 0x0A, NOT}, {"STOP", 0x0F, STOP},      \
            {"MOV", 0x10, MOV}, {"ADD", 0x11, ADD}, {"SUB", 0x12, SUB},        \
            {"MUL", 0x13, MUL}, {"DIV", 0x14, DIV}, {"CMP", 0x15, CMP},        \
            {"AND", 0x16, AND}, {"OR", 0x17, OR}, {"XOR", 0x18, XOR},          \
            {"SWAP", 0x19, SWAP}, {"SHL", 0x1A, SHL}, {"SHR", 0x1B, SHR},      \
            {"SAR", 0x1C, SAR}, {"LDL", 0x1D, LDL}, {"LDH", 0x1E, LDH}, {      \
            "RND", 0x1F, RND                                                   \
        }                                                                      \
    }

typedef struct {
    char *name;
    int16_t code;
    void (*operation)(int32_t, int32_t, type_machine);
} operatorASM;

void SYS(int32_t, int32_t, type_machine);
void JMP(int32_t, int32_t, type_machine);
void JP(int32_t, int32_t, type_machine);
void JN(int32_t, int32_t, type_machine);
void JZ(int32_t, int32_t, type_machine);
void JC(int32_t, int32_t, type_machine);
void JV(int32_t, int32_t, type_machine);
void JNP(int32_t, int32_t, type_machine);
void JNN(int32_t, int32_t, type_machine);
void JNZ(int32_t, int32_t, type_machine);
void NOT(int32_t, int32_t, type_machine);

void STOP(int32_t, int32_t, type_machine);

void MOV(int32_t, int32_t, type_machine);
void ADD(int32_t, int32_t, type_machine);
void SUB(int32_t, int32_t, type_machine);
void MUL(int32_t, int32_t, type_machine);
void DIV(int32_t, int32_t, type_machine);
void CMP(int32_t, int32_t, type_machine);
void AND(int32_t, int32_t, type_machine);
void OR(int32_t, int32_t, type_machine);
void XOR(int32_t, int32_t, type_machine);
void SWAP(int32_t, int32_t, type_machine);
void SHL(int32_t, int32_t, type_machine);
void SHR(int32_t, int32_t, type_machine);
void SAR(int32_t, int32_t, type_machine);
void LDL(int32_t, int32_t, type_machine);
void LDH(int32_t, int32_t, type_machine);
void RND(int32_t, int32_t, type_machine);