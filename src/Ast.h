#pragma once

#include "Token.h"

typedef struct Type Type;

#include <stdint.h>
#include <stdbool.h>

#define AST_KINDS                               \
    AST_KIND_BEGIN(Statement)                   \
                                                \
    AST_KIND(Scope, {                           \
        AstScope* ParentScope;                  \
        AstDeclaration** Declarations;          \
        uint64_t DeclarationCount;              \
        AstStatement** Statements;              \
        uint64_t StatementCount;                \
        uint64_t DeclarationOffset;             \
        bool Global;                            \
        bool Nested;                            \
    })                                          \
                                                \
    AST_KIND(Declaration, {                     \
        Token Name;                             \
        AstExpression* Type;                    \
        Type* ResolvedType;                     \
        Token EqualsToken;                      \
        AstExpression* Value;                   \
        bool Constant;                          \
        /* Used for the resolver */             \
        bool IsProcedureParam;                  \
        /* Used for the emitter */              \
        bool GlobalOffset;                      \
        uint64_t Offset;                        \
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
        AstStatement* ThenStatement;            \
        AstStatement* ElseStatement;            \
    })                                          \
                                                \
    AST_KIND(While, {                           \
        AstExpression* Condition;               \
        AstScope* Scope;                        \
    })                                          \
                                                \
    AST_KIND(Return, { AstExpression* Value; }) \
    AST_KIND(Break, {})                         \
    AST_KIND(Continue, {})                      \
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
    AST_KIND(Transmute, {                       \
        AstExpression* Type;                    \
        AstExpression* Expression;              \
    })                                          \
                                                \
    AST_KIND(Cast, {                            \
        AstExpression* Type;                    \
        AstExpression* Expression;              \
    })                                          \
                                                \
    AST_KIND(Integer, { Token Token; })         \
    /* AST_KIND(Float, { Token Token; }) */     \
    AST_KIND(True, {})                          \
    AST_KIND(False, {})                         \
                                                \
    AST_KIND(TypeExpression, { Type* Type; })   \
                                                \
    AST_KIND(Name, {                            \
        Token Token;                            \
        AstDeclaration* ResolvedDeclaration;    \
    })                                          \
                                                \
    AST_KIND(Procedure, {                       \
        AstDeclaration** Parameters;            \
        uint64_t ParameterCount;                \
        AstExpression* ReturnType;              \
        Type* ResolvedReturnType;               \
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
} AstKind;

#define AST_KIND_BEGIN(name)
#define AST_KIND_END(name)
#define AST_KIND(name, data) +1
enum { AstKind_Count = 0 AST_KINDS };
#undef AST_KIND
#undef AST_KIND_END
#undef AST_KIND_BEGIN

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

typedef enum Resolution {
    Resolution_Unresolved,
    Resolution_Resolving,
    Resolution_Resolved,
} Resolution;

struct Ast {
    AstKind Kind;
    Resolution Resolution;
    Type* ResolvedType;

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
