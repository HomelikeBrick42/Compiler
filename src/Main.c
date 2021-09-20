#include "VM.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

int main(int argc, char** argv) {
    uint64_t codeSize  = 1024;
    uint8_t* codeBlock = calloc(codeSize, sizeof(uint8_t));
    if (!codeBlock) {
        return EXIT_FAILURE;
    }

    {
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
    }

    VM vm;
    if (!VM_Create(&vm, codeBlock, codeSize, 1024 * 1024)) {
        return EXIT_FAILURE;
    }

    while (true) {
        if (!VM_Step(&vm)) {
            break;
        }
    }

    VM_Destroy(&vm);

    free(codeBlock);

    return EXIT_SUCCESS;
}
