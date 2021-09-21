#pragma once

#include "Token.h"

#include <stdint.h>

#define AST_KINDS                               \
    AST_KIND_BEGIN(Statement)                   \
                                                \
    AST_KIND(Scope, {                           \
        AstStatement** Statements;              \
        uint64_t StatementCount;                \
    })                                          \
                                                \
    AST_KIND(Declaration, {                     \
        Token Name;                             \
        AstExpression* Type;                    \
        Token EqualsToken;                      \
        AstExpression* Value;                   \
    })                                          \
                                                \
    AST_KIND(Assignment, {                      \
        AstExpression* Operand;                 \
        Token EqualsToken;                      \
        AstExpression* Value;                   \
    })                                          \
                                                \
    AST_KIND(If, {                              \
        AstExpression* Condition;               \
        AstScope* ThenScope;                    \
        AstScope* ElseScope;                    \
    })                                          \
                                                \
    AST_KIND(Return, { AstExpression* Value; }) \
                                                \
    AST_KIND(Print, { AstExpression* Value; })  \
                                                \
    AST_KIND_BEGIN(Expression)                  \
                                                \
    AST_KIND(Unary, {                           \
        Token Operator;                         \
        AstExpression* Operand;                 \
    })                                          \
                                                \
    AST_KIND(Binary, {                          \
        AstExpression* Left;                    \
        Token Operator;                         \
        AstExpression* Right;                   \
    })                                          \
                                                \
    AST_KIND(Name, { Token Token; })            \
    AST_KIND(Integer, { Token Token; })         \
    /* AST_KIND(Float, { Token Token; }) */     \
                                                \
    AST_KIND(Procedure, {                       \
        AstDeclaration** Parameters;            \
        uint64_t ParameterCount;                \
        AstExpression* ReturnType;              \
        AstScope* Body;                         \
    })                                          \
                                                \
    AST_KIND(Call, {                            \
        AstExpression* Operand;                 \
        AstExpression** Arguments;              \
        uint64_t ArgumentCount;                 \
    })                                          \
                                                \
    AST_KIND_END(Expression)                    \
                                                \
    AST_KIND_END(Statement)

typedef enum AstKind {
#define AST_KIND_BEGIN(name)
#define AST_KIND_END(name)
#define AST_KIND(name, data) AstKind_##name,
    AST_KINDS
#undef AST_KIND
#undef AST_KIND_END
#undef AST_KIND_BEGIN

#define AST_KIND_BEGIN(name)
#define AST_KIND_END(name)
#define AST_KIND(name, data) +1
        AstKind_Count = 0 AST_KINDS
#undef AST_KIND
#undef AST_KIND_END
#undef AST_KIND_BEGIN
} AstKind;

extern const char* AstKind_Names[AstKind_Count];

typedef struct Ast Ast;

#define AST_KIND_BEGIN(name) typedef Ast Ast##name;
#define AST_KIND_END(name)
#define AST_KIND(name, data) typedef Ast Ast##name;
AST_KINDS
#undef AST_KIND
#undef AST_KIND_END
#undef AST_KIND_BEGIN

#define AST_KIND_BEGIN(name)
#define AST_KIND_END(name)
#define AST_KIND(name, data) typedef struct data Ast##name##Data;
AST_KINDS
#undef AST_KIND
#undef AST_KIND_END
#undef AST_KIND_BEGIN

struct Ast {
    AstKind Kind;
    union {
#define AST_KIND_BEGIN(name)
#define AST_KIND_END(name)
#define AST_KIND(name, data) Ast##name##Data name;
        AST_KINDS
#undef AST_KIND
#undef AST_KIND_END
#undef AST_KIND_BEGIN
    };
};

void PrintAst(Ast* ast, uint64_t indent);
