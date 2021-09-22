#pragma once

#include <stdint.h>

#define TOKEN_KINDS                      \
    TOKEN_KIND(EndOfFile, "EndOfFile")   \
                                         \
    TOKEN_KIND(Identifier, "Identifier") \
    TOKEN_KIND(Integer, "Integer")       \
    TOKEN_KIND(Float, "Float")           \
                                         \
    TOKEN_KIND(KeywordBegin, "")         \
    TOKEN_KIND(KeywordTrue, "true")      \
    TOKEN_KIND(KeywordFalse, "false")    \
    TOKEN_KIND(KeywordIf, "if")          \
    TOKEN_KIND(KeywordElse, "else")      \
    TOKEN_KIND(KeywordReturn, "return")  \
    TOKEN_KIND(KeywordPrint, "print")    \
    TOKEN_KIND(KeywordEnd, "")           \
                                         \
    TOKEN_KIND(Semicolon, ";")           \
    TOKEN_KIND(Colon, ":")               \
    TOKEN_KIND(Comma, ",")               \
    TOKEN_KIND(OpenParenthesis, "(")     \
    TOKEN_KIND(CloseParenthesis, ")")    \
    TOKEN_KIND(OpenBrace, "{")           \
    TOKEN_KIND(CloseBrace, "}")          \
                                         \
    TOKEN_KIND(RightArrow, "->")         \
                                         \
    TOKEN_KIND(Plus, "+")                \
    TOKEN_KIND(Minus, "-")               \
    TOKEN_KIND(Asterisk, "*")            \
    TOKEN_KIND(Slash, "/")               \
    TOKEN_KIND(Equals, "=")              \
    TOKEN_KIND(Bang, "!")                \
                                         \
    TOKEN_KIND(EqualsEquals, "==")       \
    TOKEN_KIND(BangEquals, "!=")

typedef enum TokenKind {
#define TOKEN_KIND(name, str) TokenKind_##name,
    TOKEN_KINDS
#undef TOKEN_KIND
} TokenKind;

#define TOKEN_KIND(name, str) +1
enum { TokenKind_Count = 0 TOKEN_KINDS };
#undef TOKEN_KIND

extern const char* TokenKind_Names[TokenKind_Count];

typedef struct Token {
    TokenKind Kind;
    const char* FilePath;
    const char* Source;
    uint64_t Position;
    uint64_t Line;
    uint64_t Column;
    uint64_t Length;
} Token;
