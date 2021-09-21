#include "VM.h"
#include "Lexer.h"
#include "Parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

bool VM_Test();
bool Lexer_Test();
bool Parser_Test();

int main(int argc, char** argv) {
#if 0
    if (!VM_Test()) {
        return EXIT_FAILURE;
    }
#endif

#if 0
    if (!Lexer_Test()) {
        return EXIT_FAILURE;
    }
#endif

#if 1
    if (!Parser_Test()) {
        return EXIT_FAILURE;
    }
#endif

    return EXIT_SUCCESS;
}

bool VM_Test() {
    uint64_t codeSize  = 1024;
    uint8_t* codeBlock = calloc(codeSize, sizeof(uint8_t));
    if (!codeBlock) {
        return false;
    }

    {
        uint8_t* code = codeBlock;

        // Main code
        *code++          = Op_Push;
        *(uint64_t*)code = sizeof(int64_t); // Size
        code += sizeof(uint64_t);
        *(int64_t*)code = 6; // Value
        code += sizeof(int64_t);

        *code++                = Op_Call;
        uint64_t* callLocation = (uint64_t*)code; // Location
        code += sizeof(uint64_t);
        *(uint64_t*)code = sizeof(int64_t); // Arg size
        code += sizeof(uint64_t);

        *code++ = Op_PrintI64;

        *code++ = Op_Exit;

        // Setup call instruction locations
        uint64_t functionLocation = code - codeBlock;
        *callLocation             = functionLocation;

        // Function
        *code++          = Op_Load;
        *(uint64_t*)code = 0; // Offset
        code += sizeof(uint64_t);
        *(uint64_t*)code = sizeof(int64_t); // Size
        code += sizeof(uint64_t);

        *code++          = Op_Push;
        *(uint64_t*)code = sizeof(int64_t); // Size
        code += sizeof(uint64_t);
        *(int64_t*)code = 1; // Value
        code += sizeof(int64_t);

        *code++ = Op_SubI64;

        *code++          = Op_Dup;
        *(uint64_t*)code = sizeof(int64_t); // Size
        code += sizeof(uint64_t);

        *code++                    = Op_JumpZero;
        uint64_t* zeroJumpLocation = (uint64_t*)code; // Location
        code += sizeof(uint64_t);
        *(uint64_t*)code = sizeof(int64_t); // Size
        code += sizeof(uint64_t);

        *code++          = Op_Call;
        *(uint64_t*)code = functionLocation; // Location
        code += sizeof(uint64_t);
        *(uint64_t*)code = sizeof(int64_t); // Arg size
        code += sizeof(uint64_t);

        *code++          = Op_Load;
        *(uint64_t*)code = 0; // Offset
        code += sizeof(uint64_t);
        *(uint64_t*)code = sizeof(int64_t); // Size
        code += sizeof(uint64_t);

        *code++ = Op_MulI64;

        *code++          = Op_Return;
        *(uint64_t*)code = sizeof(int64_t); // Size
        code += sizeof(uint64_t);

        *zeroJumpLocation = code - codeBlock;

        *code++          = Op_Push;
        *(uint64_t*)code = sizeof(int64_t); // Size
        code += sizeof(uint64_t);
        *(int64_t*)code = 1; // Value
        code += sizeof(int64_t);

        *code++          = Op_Return;
        *(uint64_t*)code = sizeof(int64_t); // Size
        code += sizeof(uint64_t);
    }

    VM vm;
    if (!VM_Create(&vm, codeBlock, codeSize, 1024 * 1024)) {
        free(codeBlock);
        return false;
    }

    while (true) {
        if (!VM_Step(&vm)) {
            break;
        }
    }

    VM_Destroy(&vm);

    free(codeBlock);

    return true;
}

bool Lexer_Test() {
    Lexer lexer;
    if (!Lexer_Create(&lexer, "./test.lang")) {
        return false;
    }

    while (true) {
        Token token = Lexer_NextToken(&lexer);

        printf("%s\n", TokenKind_Names[token.Kind]);

        if (token.Kind == TokenKind_EndOfFile) {
            break;
        }
    }

    bool success = !lexer.WasError;

    Lexer_Destroy(&lexer);

    return success;
}

bool Parser_Test() {
    Parser parser;
    if (!Parser_Create(&parser, "./test.lang")) {
        return false;
    }

    AstScope* scope = Parser_ParseFile(&parser);
    (void)scope;

    bool success = !parser.WasError && !parser.Lexer.WasError;

    Parser_Destroy(&parser);

    return success;
}
