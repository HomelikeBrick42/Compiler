#pragma once

#include "Token.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct Lexer {
    char* FilePath;
    char* Source;
    uint64_t SourceLength;
    uint64_t Position;
    uint64_t Line;
    uint64_t Column;
    char Current;
    bool WasError;
} Lexer;

bool Lexer_Create(Lexer* lexer, const char* path);
void Lexer_Destroy(Lexer* lexer);
Token Lexer_NextToken(Lexer* lexer);

char Lexer_NextChar(Lexer* lexer);
bool Lexer_ExpectChar(Lexer* lexer, char expected);
