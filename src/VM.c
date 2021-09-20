#include "VM.h"

#include <stdlib.h>
#include <stdio.h>

bool VM_Create(VM* vm, uint8_t* code, uint64_t codeSize, uint64_t stackSize) {
    if (!vm) {
        return false;
    }

    *vm = (VM){};

    vm->Code     = code;
    vm->CodeSize = codeSize;
    vm->Ip       = vm->Code;

    vm->Stack = malloc(stackSize);
    if (!vm->Stack) {
        return false;
    }

    vm->StackSize = stackSize;
    vm->Sp = vm->Stack;
    vm->Bp = vm->Stack;

    return true;
}

void VM_Destroy(VM* vm) {
    free(vm->Stack);
}

bool VM_Step(VM* vm) {
    if (vm->Ip - vm->Code < 0 || (uint64_t)(vm->Ip - vm->Code) >= vm->CodeSize) {
        fprintf(stderr, "Instruction out of range\n");
        return false;
    }

    if (vm->Sp - vm->Stack < 0 || (uint64_t)(vm->Sp - vm->Stack) >= vm->StackSize) {
        fprintf(stderr, "Stack pointer out of range\n");
        return false;
    }

    if (vm->Bp - vm->Stack < 0 || (uint64_t)(vm->Bp - vm->Stack) >= vm->StackSize) {
        fprintf(stderr, "Base stack pointer out of range\n");
        return false;
    }

    switch ((Op)*vm->Ip++) {
        default:
        case Op_Invalid: {
            fflush(stdout);
            fprintf(stderr, "Invalid instruction\n");
            return false;
        } break;

        case Op_Exit: {
            return false;
        } break;

        case Op_Push: {
            uint64_t size = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            for (uint64_t i = 0; i < size; i++) {
                *vm->Sp++ = *vm->Ip++;
            }
        } break;

        case Op_Pop: {
            uint64_t size = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            vm->Sp -= size;
        } break;

        case Op_AllocStack: {
            uint64_t size = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);
            vm->Sp += size;
        } break;

        case Op_Dup: {
            uint64_t size = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            for (uint64_t i = 0; i < size; i++) {
                vm->Sp[i] = vm->Sp[(int64_t)i - (int64_t)size];
            }
            vm->Sp += size;
        } break;

#define MATH_OP(inst, type, op)  \
    case inst: {                 \
        vm->Sp -= sizeof(type);  \
        type b = *(type*)vm->Sp; \
                                 \
        vm->Sp -= sizeof(type);  \
        type a = *(type*)vm->Sp; \
                                 \
        *(type*)vm->Sp = a op b; \
        vm->Sp += sizeof(type);  \
    } break

            MATH_OP(Op_AddI64, int64_t, +);
            MATH_OP(Op_AddU64, uint64_t, +);
            MATH_OP(Op_SubI64, int64_t, -);
            MATH_OP(Op_SubU64, uint64_t, -);
            MATH_OP(Op_MulI64, int64_t, *);
            MATH_OP(Op_MulU64, uint64_t, *);
            MATH_OP(Op_DivI64, int64_t, /);
            MATH_OP(Op_DivU64, uint64_t, /);

#undef MATH_OP

        case Op_PrintI64: {
            vm->Sp -= sizeof(int64_t);
            int64_t value = *(int64_t*)vm->Sp;

            printf("%lld\n", value);
        } break;

        case Op_PrintU64: {
            vm->Sp -= sizeof(uint64_t);
            uint64_t value = *(uint64_t*)vm->Sp;

            printf("%llu\n", value);
        } break;

        case Op_Equal: {
            uint64_t size = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            uint8_t buffer[size];

            vm->Sp -= size;
            for (uint64_t i = 0; i < size; i++) {
                buffer[i] = vm->Sp[i];
            }

            bool equal = true;
            vm->Sp -= size;
            for (uint64_t i = 0; i < size; i++) {
                if (vm->Sp[i] != buffer[i]) {
                    equal = false;
                    break;
                }
            }

            *vm->Sp++ = equal ? 1 : 0;
        } break;

        case Op_Jump: {
            uint64_t location = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);
            vm->Ip = &vm->Code[location];
        } break;

        case Op_JumpZero: {
            uint64_t location = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            uint64_t size = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            vm->Sp -= size;

            bool zero = true;
            for (uint64_t i = 0; i < size; i++) {
                if (vm->Sp[i] != 0) {
                    zero = false;
                    break;
                }
            }

            if (zero) {
                vm->Ip = &vm->Code[location];
            }
        } break;

        case Op_JumpNonZero: {
            uint64_t location = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            uint64_t size = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            vm->Sp -= size;

            bool zero = true;
            for (uint64_t i = 0; i < size; i++) {
                if (vm->Sp[i] != 0) {
                    zero = false;
                    break;
                }
            }

            if (!zero) {
                vm->Ip = &vm->Code[location];
            }
        } break;

        case Op_Call: {
            uint64_t location = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            uint64_t argSize = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            // TODO: Is there a replacement for this?
            uint8_t argBuffer[argSize];

            vm->Sp -= argSize;
            for (uint64_t i = 0; i < argSize; i++) {
                argBuffer[i] = vm->Sp[i];
            }

            *(uint64_t*)vm->Sp = vm->Ip - vm->Code;
            vm->Sp += sizeof(uint64_t);

            *(uint8_t**)vm->Sp = vm->Bp;
            vm->Sp += sizeof(uint8_t*);

            vm->Bp = vm->Sp;
            vm->Ip = &vm->Code[location];

            for (uint64_t i = 0; i < argSize; i++) {
                vm->Sp[i] = argBuffer[i];
            }
            vm->Sp += argSize;
        } break;

        case Op_Return: {
            uint64_t returnSize = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            // TODO: Is there a replacement for this?
            uint8_t returnBuffer[returnSize];

            vm->Sp -= returnSize;
            for (uint64_t i = 0; i < returnSize; i++) {
                returnBuffer[i] = vm->Sp[i];
            }

            vm->Sp = vm->Bp;

            vm->Sp -= sizeof(uint8_t*);
            vm->Bp = *(uint8_t**)vm->Sp;

            vm->Sp -= sizeof(uint64_t);
            uint64_t location = *(uint64_t*)vm->Sp;

            vm->Ip = &vm->Code[location];

            for (uint64_t i = 0; i < returnSize; i++) {
                vm->Sp[i] = returnBuffer[i];
            }
            vm->Sp += returnSize;
        } break;

        case Op_Load: {
            uint64_t offset = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            uint64_t size = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            for (uint64_t i = 0; i < size; i++) {
                vm->Sp[i] = vm->Bp[offset + i];
            }
            vm->Sp += size;
        } break;

        case Op_Store: {
            uint64_t offset = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            uint64_t size = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            vm->Sp -= size;
            for (uint64_t i = 0; i < size; i++) {
                vm->Bp[offset + i] = vm->Sp[i];
            }
        } break;
    }

    return true;
}
