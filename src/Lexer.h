#pragma once

#include "Token.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct Lexer {
    String FilePath;
    String Source;
    uint64_t Position;
    uint64_t Line;
    uint64_t Column;
    uint8_t Current;
    bool Error;
} Lexer;

Lexer Lexer_Create(String filePath, String source);

uint8_t Lexer_NextChar(Lexer* lexer);
uint8_t Lexer_ExpectChar(Lexer* lexer, uint8_t chr);

Token Lexer_NextToken(Lexer* lexer);
