#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

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

bool Execute(uint8_t* codeBlock, uint64_t codeSize) {
    uint8_t* ip = codeBlock;

    void* stack = malloc(1024 * 1024);
    if (!stack) {
        return false;
    }

    uint8_t* sp = stack;
    uint8_t* bp = sp;

    while (true) {
        if ((uint64_t)(ip - codeBlock) >= codeSize) {
            free(stack);
            return false;
        }

        switch ((Op)*ip++) {
            default:
            case Op_Invalid: {
                free(stack);
                return false;
            } break;

            case Op_Exit: {
                goto Exit;
            } break;

            case Op_Push: {
                uint64_t size = *(uint64_t*)ip;
                ip += sizeof(uint64_t);

                for (uint64_t i = 0; i < size; i++) {
                    *sp++ = *ip++;
                }
            } break;

            case Op_Pop: {
                uint64_t size = *(uint64_t*)ip;
                ip += sizeof(uint64_t);

                sp -= size;
            } break;

            case Op_AllocStack: {
                uint64_t size = *(uint64_t*)ip;
                ip += sizeof(uint64_t);
                sp += size;
            } break;

            case Op_Dup: {
                uint64_t size = *(uint64_t*)ip;
                ip += sizeof(uint64_t);

                for (uint64_t i = 0; i < size; i++) {
                    sp[i] = sp[(int64_t)i - (int64_t)size];
                }
                sp += size;
            } break;

#define MATH_OP(inst, type, op) \
    case inst: {                \
        sp -= sizeof(type);     \
        type b = *(type*)sp;    \
                                \
        sp -= sizeof(type);     \
        type a = *(type*)sp;    \
                                \
        *(type*)sp = a op b;    \
        sp += sizeof(type);     \
    } break

                MATH_OP(Op_AddI64, int64_t, +);
                MATH_OP(Op_AddU64, uint64_t, +);
                MATH_OP(Op_SubI64, int64_t, -);
                MATH_OP(Op_SubU64, uint64_t, -);

#undef MATH_OP

            case Op_PrintI64: {
                sp -= sizeof(int64_t);
                int64_t value = *(int64_t*)sp;

                printf("%lld\n", value);
            } break;

            case Op_PrintU64: {
                sp -= sizeof(uint64_t);
                uint64_t value = *(uint64_t*)sp;

                printf("%llu\n", value);
            } break;

            case Op_Jump: {
                uint64_t location = *(uint64_t*)ip;
                ip += sizeof(uint64_t);
                ip = &codeBlock[location];
            } break;

            case Op_JumpZero: {
                uint64_t location = *(uint64_t*)ip;
                ip += sizeof(uint64_t);

                uint64_t size = *(uint64_t*)ip;
                ip += sizeof(uint64_t);

                sp -= size;

                bool zero = true;
                for (uint64_t i = 0; i < size; i++) {
                    if (sp[i] != 0) {
                        zero = false;
                        break;
                    }
                }

                if (zero) {
                    ip = &codeBlock[location];
                }
            } break;

            case Op_JumpNonZero: {
                uint64_t location = *(uint64_t*)ip;
                ip += sizeof(uint64_t);

                uint64_t size = *(uint64_t*)ip;
                ip += sizeof(uint64_t);

                sp -= size;

                bool zero = true;
                for (uint64_t i = 0; i < size; i++) {
                    if (sp[i] != 0) {
                        zero = false;
                        break;
                    }
                }

                if (!zero) {
                    ip = &codeBlock[location];
                }
            } break;

            case Op_Call: {
                uint64_t location = *(uint64_t*)ip;
                ip += sizeof(uint64_t);

                uint64_t argSize = *(uint64_t*)ip;
                ip += sizeof(uint64_t);

                // TODO: Is there a replacement for this?
                uint8_t argBuffer[argSize];

                sp -= argSize;
                for (uint64_t i = 0; i < argSize; i++) {
                    argBuffer[i] = sp[i];
                }

                *(uint64_t*)sp = ip - codeBlock;
                sp += sizeof(uint64_t);

                *(uint8_t**)sp = bp;
                sp += sizeof(uint8_t*);

                bp = sp;
                ip = &codeBlock[location];

                for (uint64_t i = 0; i < argSize; i++) {
                    sp[i] = argBuffer[i];
                }
                sp += argSize;
            } break;

            case Op_Return: {
                sp = bp;

                sp -= sizeof(uint8_t*);
                bp = *(uint8_t**)sp;

                sp -= sizeof(uint64_t);
                uint64_t location = *(uint64_t*)sp;

                ip = &codeBlock[location];
            } break;

            case Op_Load: {
                uint64_t offset = *(uint64_t*)ip;
                ip += sizeof(uint64_t);

                uint64_t size = *(uint64_t*)ip;
                ip += sizeof(uint64_t);

                for (uint64_t i = 0; i < size; i++) {
                    sp[i] = bp[offset + i];
                }
                sp += size;
            } break;

            case Op_Store: {
                uint64_t offset = *(uint64_t*)ip;
                ip += sizeof(uint64_t);

                uint64_t size = *(uint64_t*)ip;
                ip += sizeof(uint64_t);

                sp -= size;
                for (uint64_t i = 0; i < size; i++) {
                    bp[offset + i] = sp[i];
                }
            } break;
        }
    }

Exit:
    free(stack);
    return true;
}

int main(int argc, char** argv) {
    uint64_t codeSize  = 1024;
    uint8_t* codeBlock = calloc(codeSize, sizeof(uint8_t));
    if (!codeBlock) {
        return EXIT_FAILURE;
    }

    uint8_t* code = codeBlock;

    // Main code
    *code++          = Op_Push;
    *(uint64_t*)code = sizeof(int64_t); // Size
    code += sizeof(uint64_t);
    *(int64_t*)code = 21; // Value
    code += sizeof(int64_t);

    *code++                 = Op_Call;
    uint64_t* callLocation1 = (uint64_t*)code; // Location
    code += sizeof(uint64_t);
    *(uint64_t*)code = sizeof(int64_t); // Arg size
    code += sizeof(uint64_t);

    *code++          = Op_Push;
    *(uint64_t*)code = sizeof(int64_t); // Size
    code += sizeof(uint64_t);
    *(int64_t*)code = 6; // Value
    code += sizeof(int64_t);

    *code++                 = Op_Call;
    uint64_t* callLocation2 = (uint64_t*)code; // Location
    code += sizeof(uint64_t);
    *(uint64_t*)code = sizeof(int64_t); // Arg size
    code += sizeof(uint64_t);

    *code++ = Op_Exit;

    // Setup call instruction locations
    uint64_t functionLocation = code - codeBlock;
    *callLocation1            = functionLocation;
    *callLocation2            = functionLocation;

    // Function
    *code++          = Op_Load;
    *(uint64_t*)code = 0; // Offset
    code += sizeof(uint64_t);
    *(uint64_t*)code = sizeof(int64_t); // Size
    code += sizeof(uint64_t);

    *code++          = Op_Dup;
    *(uint64_t*)code = sizeof(int64_t); // Size
    code += sizeof(uint64_t);

    *code++ = Op_AddI64;

    *code++ = Op_PrintI64;

    *code++ = Op_Return;

    if (!Execute(codeBlock, codeSize)) {
        return EXIT_FAILURE;
    }

    free(codeBlock);

    return EXIT_SUCCESS;
}
