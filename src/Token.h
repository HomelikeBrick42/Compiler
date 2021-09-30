#pragma once

#include "Strings.h"

#define TOKEN_KINDS                            \
    TOKEN_KIND(Error, "Error")                 \
    TOKEN_KIND(EndOfFile, "EndOfFile")         \
                                               \
    TOKEN_KIND(Identifier, "Identifier")       \
    TOKEN_KIND(Integer, "Integer")             \
    TOKEN_KIND(Float, "Float")                 \
    TOKEN_KIND(String, "String")               \
                                               \
    TOKEN_KIND_BEGIN(HashDirective)            \
    TOKEN_KIND(CompilerProc, "#compiler_proc") \
    TOKEN_KIND_END(HashDirective)              \
                                               \
    TOKEN_KIND_BEGIN(Keyword)                  \
    TOKEN_KIND(True, "true")                   \
    TOKEN_KIND(False, "false")                 \
    TOKEN_KIND(If, "if")                       \
    TOKEN_KIND(Do, "do")                       \
    TOKEN_KIND(Else, "else")                   \
    TOKEN_KIND(While, "while")                 \
    TOKEN_KIND(Break, "break")                 \
    TOKEN_KIND(Continue, "continue")           \
    TOKEN_KIND(Return, "return")               \
    TOKEN_KIND(Print, "print")                 \
    TOKEN_KIND(Cast, "cast")                   \
    TOKEN_KIND(Transmute, "transmute")         \
    TOKEN_KIND(TypeOf, "typeof")               \
    TOKEN_KIND(SizeOf, "sizeof")               \
    TOKEN_KIND(Struct, "struct")               \
    TOKEN_KIND_END(Keyword)                    \
                                               \
    TOKEN_KIND(Semicolon, ";")                 \
    TOKEN_KIND(Colon, ":")                     \
    TOKEN_KIND(Comma, ",")                     \
    TOKEN_KIND(Period, ".")                    \
    TOKEN_KIND(OpenParenthesis, "(")           \
    TOKEN_KIND(CloseParenthesis, ")")          \
    TOKEN_KIND(OpenBrace, "{")                 \
    TOKEN_KIND(CloseBrace, "}")                \
                                               \
    TOKEN_KIND(RightArrow, "->")               \
                                               \
    TOKEN_KIND_BEGIN(Operator)                 \
    TOKEN_KIND(Plus, "+")                      \
    TOKEN_KIND(Minus, "-")                     \
    TOKEN_KIND(Asterisk, "*")                  \
    TOKEN_KIND(Slash, "/")                     \
    TOKEN_KIND(Percent, "%")                   \
    TOKEN_KIND(LessThan, "<")                  \
    TOKEN_KIND(GreaterThan, ">")               \
    TOKEN_KIND(Bang, "!")                      \
    TOKEN_KIND(LessThanEquals, "<=")           \
    TOKEN_KIND(GreaterThanEquals, ">=")        \
    TOKEN_KIND(EqualsEquals, "==")             \
    TOKEN_KIND(BangEquals, "!=")               \
    TOKEN_KIND_END(Operator)                   \
                                               \
    TOKEN_KIND_BEGIN(Assignment)               \
    TOKEN_KIND(Equals, "=")                    \
    TOKEN_KIND(PlusEquals, "+=")               \
    TOKEN_KIND(MinusEquals, "-=")              \
    TOKEN_KIND(AsteriskEquals, "*=")           \
    TOKEN_KIND(SlashEquals, "/=")              \
    TOKEN_KIND(PercentEquals, "%=")            \
    TOKEN_KIND_END(Assignment)

typedef enum TokenKind {
#define TOKEN_KIND(name, str)  TokenKind_##name,
#define TOKEN_KIND_BEGIN(name) TokenKind_Begin_##name,
#define TOKEN_KIND_END(name)   TokenKind_End_##name,
    TOKEN_KINDS
#undef TOKEN_KIND
#undef TOKEN_KIND_BEGIN
#undef TOKEN_KIND_END
} TokenKind;

#define TOKEN_KIND(name, str)  +1
#define TOKEN_KIND_BEGIN(name) +1
#define TOKEN_KIND_END(name)   +1
enum { TokenKind_Count = 0 TOKEN_KINDS };
#undef TOKEN_KIND
#undef TOKEN_KIND_BEGIN
#undef TOKEN_KIND_END

extern String TokenKind_Names[TokenKind_Count];

typedef struct Token {
    TokenKind Kind;
    String FilePath;
    String Source;
    uint64_t Position;
    uint64_t Column;
    uint64_t Line;
    uint64_t Length;
    union {
        uint64_t IntValue;
        double FloatValue;
        String StringValue;
    };
} Token;

#define TOKEN_KIND(name, str)
#define TOKEN_KIND_BEGIN(name) bool Token_Is##name(Token token);
#define TOKEN_KIND_END(name)
TOKEN_KINDS
#undef TOKEN_KIND
#undef TOKEN_KIND_BEGIN
#undef TOKEN_KIND_END

void Token_Error(Token token, const char* message, ...);
