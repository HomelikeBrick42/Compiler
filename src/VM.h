#pragma once

#include <stdint.h>
#include <stdbool.h>

#define OPS                                                                                                                      \
    OP(Invalid)                                                                                                                  \
    OP(Exit)                                                                                                                     \
    OP(Push)                                                                                                                     \
    OP(Pop)                                                                                                                      \
    OP(Dup)                                                                                                                      \
    OP(AddI64)                                                                                                                   \
    OP(AddU64)                                                                                                                   \
    OP(SubI64)                                                                                                                   \
    OP(SubU64)                                                                                                                   \
    OP(MulI64)                                                                                                                   \
    OP(MulU64)                                                                                                                   \
    OP(DivI64)                                                                                                                   \
    OP(DivU64)                                                                                                                   \
    OP(PrintI64)                                                                                                                 \
    OP(PrintU64)                                                                                                                 \
    OP(Equal)                                                                                                                    \
    OP(Jump)                                                                                                                     \
    OP(JumpZero)                                                                                                                 \
    OP(JumpNonZero)                                                                                                              \
    OP(Call)                                                                                                                     \
    OP(Return)                                                                                                                   \
    OP(LoadRelative) /* TODO: For arrays make a version which takes the offset (or maybe a offset for the offset) from the stack \
                      */                                                                                                         \
    OP(StoreRelative) /* TODO: Same as ^ */                                                                                      \
    OP(LoadAbsolute)  /* TODO: Same as ^ */                                                                                      \
    OP(StoreAbsolute) /* TODO: Same as ^ */

typedef enum Op {
#define OP(name) Op_##name,
    OPS
#undef OP
} Op;

#define OP(name) +1
enum { Op_Count = 0 OPS };
#undef OP

extern const char* Op_Names[Op_Count];

typedef struct VM {
    uint8_t* Code;
    uint64_t CodeSize;
    uint8_t* Ip;
    uint8_t* Stack;
    uint64_t StackSize;
    uint8_t* Sp;
    uint8_t* Bp;
} VM;

bool VM_Create(VM* vm, uint8_t* code, uint64_t codeSize, uint64_t stackSize);
void VM_Destroy(VM* vm);
bool VM_Step(VM* vm);

void PrintBytecode(uint8_t* code, uint64_t codeSize);
