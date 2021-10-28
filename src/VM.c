#include "VM.h"

#include <stdlib.h>
#include <stdio.h>

const char* Op_Names[Op_Count] = {
#define OP(name) [Op_##name] = #name,
    OPS
#undef OP
};

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
    vm->Sp        = vm->Stack;
    vm->Bp        = vm->Stack;

    return true;
}

void VM_Destroy(VM* vm) {
    free(vm->Stack);
}

bool VM_Step(VM* vm) {
    if (vm->Ip - vm->Code < 0 || (uint64_t)(vm->Ip - vm->Code) >= vm->CodeSize) {
        fprintf(stderr,
                "Instruction out of range Sp: %lld Bp: %lld Ip: %lld\n",
                vm->Sp - vm->Stack,
                vm->Bp - vm->Stack,
                vm->Ip - vm->Code);
        return false;
    }

    if (vm->Sp - vm->Stack < 0 || (uint64_t)(vm->Sp - vm->Stack) >= vm->StackSize) {
        fprintf(stderr,
                "Stack pointer out of range Sp: %lld Bp: %lld Ip: %lld\n",
                vm->Sp - vm->Stack,
                vm->Bp - vm->Stack,
                vm->Ip - vm->Code);
        return false;
    }

    if (vm->Bp - vm->Stack < 0 || (uint64_t)(vm->Bp - vm->Stack) >= vm->StackSize) {
        fprintf(stderr,
                "Base stack pointer out of range Sp: %lld Bp: %lld Ip: %lld\n",
                vm->Sp - vm->Stack,
                vm->Bp - vm->Stack,
                vm->Ip - vm->Code);
        return false;
    }

#define SHOW_DEBUG_INFO 0

#if SHOW_DEBUG_INFO
    printf("%05llu %s\n", vm->Ip - vm->Code, Op_Names[*vm->Ip]);
#endif

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

        case Op_AllocStack: {
            uint64_t size = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            for (uint64_t i = 0; i < size; i++) {
                *vm->Sp++ = 0;
            }
        } break;

        case Op_Pop: {
            uint64_t size = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            vm->Sp -= size;
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

        case Op_NegateBool: {
            vm->Sp[-1] = !vm->Sp[-1];
        } break;

        case Op_NegateI64: {
            vm->Sp -= sizeof(int64_t);
            int64_t value = *(int64_t*)vm->Sp;

            *(int64_t*)vm->Sp = -value;
            vm->Sp += sizeof(int64_t);
        } break;

        case Op_NegateU64: {
            vm->Sp -= sizeof(uint64_t);
            uint64_t value = *(uint64_t*)vm->Sp;

            *(uint64_t*)vm->Sp = -value;
            vm->Sp += sizeof(uint64_t);
        } break;

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

        case Op_PrintBool: {
            vm->Sp -= sizeof(uint8_t);
            uint8_t value = *(uint8_t*)vm->Sp;

            printf("%s\n", value ? "true" : "false");
        } break;

        case Op_I64ToU64: {
            vm->Sp -= sizeof(int64_t);
            int64_t value = *(int64_t*)vm->Sp;

            *(uint64_t*)vm->Sp = (uint64_t)value;
            vm->Sp += sizeof(uint64_t);
        } break;

        case Op_U64ToI64: {
            vm->Sp -= sizeof(uint64_t);
            uint64_t value = *(uint64_t*)vm->Sp;

            *(int64_t*)vm->Sp = (int64_t)value;
            vm->Sp += sizeof(int64_t);
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
            uint64_t argSize = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            // TODO: Is there a replacement for this?
            uint8_t argBuffer[argSize];

            vm->Sp -= argSize;
            for (uint64_t i = 0; i < argSize; i++) {
                argBuffer[i] = vm->Sp[i];
            }

            vm->Sp -= sizeof(uint64_t);
            uint64_t location = *(uint64_t*)vm->Sp;

            *(uint64_t*)vm->Sp = vm->Ip - vm->Code;
            vm->Sp += sizeof(uint64_t);

            *(int64_t*)vm->Sp = vm->Bp - vm->Stack;
            vm->Sp += sizeof(int64_t);

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

            vm->Sp -= sizeof(int64_t);
            vm->Bp = &vm->Stack[*(int64_t*)vm->Sp];

            vm->Sp -= sizeof(uint64_t);
            uint64_t location = *(uint64_t*)vm->Sp;

            vm->Ip = &vm->Code[location];

            for (uint64_t i = 0; i < returnSize; i++) {
                vm->Sp[i] = returnBuffer[i];
            }
            vm->Sp += returnSize;
        } break;

        case Op_LoadRelative: {
            uint64_t offset = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            uint64_t size = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

#if SHOW_DEBUG_INFO
            if (size == 8) {
                printf("Value: %llu\n", *(uint64_t*)&vm->Bp[offset]);
            }
#endif

            for (uint64_t i = 0; i < size; i++) {
                vm->Sp[i] = vm->Bp[offset + i];
            }

#if SHOW_DEBUG_INFO
            if (size == 8) {
                printf("Value: %llu\n", *(uint64_t*)vm->Sp);
            }
#endif

            vm->Sp += size;
        } break;

        case Op_StoreRelative: {
            uint64_t offset = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            uint64_t size = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            vm->Sp -= size;

#if SHOW_DEBUG_INFO
            if (size == 8) {
                printf("Value: %llu\n", *(uint64_t*)vm->Sp);
            }
#endif

            for (uint64_t i = 0; i < size; i++) {
                vm->Bp[offset + i] = vm->Sp[i];
            }

#if SHOW_DEBUG_INFO
            if (size == 8) {
                printf("Value: %llu\n", *(uint64_t*)&vm->Bp[offset]);
            }
#endif
        } break;

        case Op_LoadAbsolute: {
            uint64_t offset = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            uint64_t size = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

#if SHOW_DEBUG_INFO
            if (size == 8) {
                printf("Value: %llu\n", *(uint64_t*)&vm->Stack[offset]);
            }
#endif

            for (uint64_t i = 0; i < size; i++) {
                vm->Sp[i] = vm->Stack[offset + i];
            }

#if SHOW_DEBUG_INFO
            if (size == 8) {
                printf("Value: %llu\n", *(uint64_t*)vm->Sp);
            }
#endif

            vm->Sp += size;
        } break;

        case Op_StoreAbsolute: {
            uint64_t offset = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            uint64_t size = *(uint64_t*)vm->Ip;
            vm->Ip += sizeof(uint64_t);

            vm->Sp -= size;

#if SHOW_DEBUG_INFO
            if (size == 8) {
                printf("Value: %llu\n", *(uint64_t*)vm->Sp);
            }
#endif

            for (uint64_t i = 0; i < size; i++) {
                vm->Stack[offset + i] = vm->Sp[i];
            }

#if SHOW_DEBUG_INFO
            if (size == 8) {
                printf("Value: %llu\n", *(uint64_t*)&vm->Stack[offset]);
            }
#endif
        } break;
    }

#if SHOW_DEBUG_INFO
    printf("Stack: [");
    for (uint64_t* it = (uint64_t*)vm->Stack; it < (uint64_t*)vm->Sp; it++) {
        printf("%llu", *it);
        if (it < (uint64_t*)vm->Sp - 1) {
            printf(", ");
        }
    }
    printf("]\n\n");
#endif

    return true;
}

void PrintBytecode(uint8_t* code, uint64_t codeSize) {
    uint8_t* ip = code;
    while ((uint64_t)(ip - code) < codeSize) {
        Op inst = *ip;
        printf("%05llu %s", ip - code, inst < Op_Count ? Op_Names[inst] : "Invalid");
        ip++;
        switch (inst) {
            case Op_Push: {
                uint64_t size = *(uint64_t*)ip;
                ip += sizeof(uint64_t);
                ip += size;
                printf(": Size = %llu", size);
                if (size == 8) {
                    printf(", Value: %llu", *(uint64_t*)(ip - size));
                }
            } break;

            case Op_AllocStack:
            case Op_Pop:
            case Op_Dup:
            case Op_Equal: {
                uint64_t size = *(uint64_t*)ip;
                ip += sizeof(uint64_t);
                printf(": Size = %llu", size);
            } break;

            case Op_Jump: {
                uint64_t location = *(uint64_t*)ip;
                ip += sizeof(uint64_t);
                printf(": Location = %llu", location);
            } break;

            case Op_JumpZero:
            case Op_JumpNonZero: {
                uint64_t location = *(uint64_t*)ip;
                ip += sizeof(uint64_t);
                uint64_t size = *(uint64_t*)ip;
                ip += sizeof(uint64_t);
                printf(": Location = %llu, Size = %llu", location, size);
            } break;

            case Op_Call: {
                uint64_t argSize = *(uint64_t*)ip;
                ip += sizeof(uint64_t);
                printf(": ArgSize = %llu", argSize);
            } break;

            case Op_Return: {
                uint64_t returnSize = *(uint64_t*)ip;
                ip += sizeof(uint64_t);
                printf(": ReturnSize = %llu", returnSize);
            } break;

            case Op_LoadRelative:
            case Op_StoreRelative:
            case Op_LoadAbsolute:
            case Op_StoreAbsolute: {
                uint64_t offset = *(uint64_t*)ip;
                ip += sizeof(uint64_t);

                uint64_t size = *(uint64_t*)ip;
                ip += sizeof(uint64_t);

                printf(": Offset = %llu, Size = %llu", offset, size);
            } break;

            default: {
            } break;
        }
        putchar('\n');
    }
}
