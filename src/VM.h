#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum Op {
    Op_Invalid,
    Op_Exit,
    Op_Push,
    Op_Pop,
    Op_AllocStack,
    Op_Dup,
    Op_AddI64,
    Op_AddU64,
    Op_SubI64,
    Op_SubU64,
    Op_PrintI64,
    Op_PrintU64,
    Op_Jump,
    Op_JumpZero,
    Op_JumpNonZero,
    Op_Call,
    Op_Return,
    Op_Load, // TODO: For arrays make a version which takes the offset (or maybe a offset for the offset) from the stack
    Op_Store, // TODO: Same as ^
} Op;

typedef struct VM {
    uint8_t* Code;
    uint64_t CodeSize;
    uint8_t* Ip;
    uint8_t* Sp;
    uint8_t* Bp;
} VM;

bool VM_Create(VM* vm, uint8_t* code, uint64_t codeSize, uint64_t stackSize);
void VM_Destroy(VM* vm);
bool VM_Step(VM* vm);
