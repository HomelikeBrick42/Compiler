#pragma once

#include "Array.h"
#include "Token.h"

#include <stdint.h>
#include <stdbool.h>

#define AST_KINDS                                                 \
    AST_KIND(Invalid, {})                                         \
                                                                  \
    AST_KIND_BEGIN(Statement, { AstScope* ParentScope; })         \
                                                                  \
    AST_KIND(InvalidStatement, {})                                \
                                                                  \
    AST_KIND(Scope, {                                             \
        Token OpenBrace;                                          \
        Token CloseBrace;                                         \
        AstStatementArray Statements;                             \
        bool Global;                                              \
        AstProcedure* ParentProcedure;                            \
    })                                                            \
                                                                  \
    AST_KIND(Declaration, {                                       \
        Token Name;                                               \
        Token ColonToken;                                         \
        AstExpression* Type;                                      \
        AstType* ResolvedType;                                    \
        Token EqualsOrColonToken;                                 \
        AstExpression* Value;                                     \
        AstProcedure* ParentProcedure;                            \
        bool Constant;                                            \
    })                                                            \
                                                                  \
    AST_KIND(Assignment, {                                        \
        AstExpression* Operand;                                   \
        Token EqualsToken;                                        \
        AstExpression* Value;                                     \
    })                                                            \
                                                                  \
    AST_KIND(If, {                                                \
        Token IfToken;                                            \
        AstExpression* Condition;                                 \
        AstStatement* ThenStatement;                              \
        Token ElseToken;                                          \
        AstStatement* ElseStatement;                              \
    })                                                            \
                                                                  \
    AST_KIND(While, {                                             \
        Token WhileToken;                                         \
        AstExpression* Condition;                                 \
        AstStatement* Statement;                                  \
    })                                                            \
                                                                  \
    AST_KIND(Return, {                                            \
        Token ReturnToken;                                        \
        AstExpression* Value;                                     \
    })                                                            \
                                                                  \
    AST_KIND(StatementExpression, { AstExpression* Expression; }) \
                                                                  \
    AST_KIND(Semicolon, {})                                       \
                                                                  \
    AST_KIND_END(Statement)                                       \
                                                                  \
    AST_KIND_BEGIN(Expression, {                                  \
        AstStatement* ParentStatement;                            \
        AstType* ResolvedType;                                    \
    })                                                            \
                                                                  \
    AST_KIND(InvalidExpression, {})                               \
                                                                  \
    AST_KIND(Unary, {                                             \
        Token Operator;                                           \
        AstExpression* Operand;                                   \
    })                                                            \
                                                                  \
    AST_KIND(Binary, {                                            \
        AstExpression* Left;                                      \
        Token Operator;                                           \
        AstExpression* Right;                                     \
    })                                                            \
                                                                  \
    AST_KIND(Cast, {                                              \
        Token CastToken;                                          \
        AstExpression* Type;                                      \
        AstExpression* Expression;                                \
    })                                                            \
                                                                  \
    AST_KIND(Transmute, {                                         \
        Token TransmuteToken;                                     \
        AstExpression* Type;                                      \
        AstExpression* Expression;                                \
    })                                                            \
                                                                  \
    AST_KIND(TypeOf, {                                            \
        Token TypeOfToken;                                        \
        AstExpression* Expression;                                \
    })                                                            \
                                                                  \
    AST_KIND(SizeOf, {                                            \
        Token SizeOfToken;                                        \
        AstExpression* Expression;                                \
    })                                                            \
                                                                  \
    AST_KIND(Integer, { Token Token; })                           \
    AST_KIND(Float, { Token Token; })                             \
    AST_KIND(String, { Token Token; })                            \
                                                                  \
    AST_KIND(Name, {                                              \
        Token Token;                                              \
        AstDeclaration* ResolvedDeclaration;                      \
    })                                                            \
                                                                  \
    AST_KIND(Procedure, {                                         \
        Token OpenParenthesis;                                    \
        Token CloseParenthesis;                                   \
        AstDeclarationArray Parameters;                           \
        AstExpression* ReturnType;                                \
        AstScope* Body;                                           \
        bool CompilerProc;                                        \
        union {                                                   \
            Token CompilerProcString;                             \
        } Data;                                                   \
    })                                                            \
                                                                  \
    AST_KIND(Call, {                                              \
        Token OpenParenthesis;                                    \
        Token CloseParenthesis;                                   \
        AstExpressionArray Arguments;                             \
        AstExpression* Operand;                                   \
    })                                                            \
                                                                  \
    AST_KIND(BuitinType, { Token BuiltinToken; })                 \
                                                                  \
    AST_KIND_END(Expression)                                      \
                                                                  \
    AST_KIND_BEGIN(Type, { uint64_t Size; })                      \
                                                                  \
    AST_KIND(InvalidType, {})                                     \
                                                                  \
    AST_KIND(TypeType, {})                                        \
                                                                  \
    AST_KIND(TypeInteger, { bool Signed; })                       \
    AST_KIND(TypeFloat, {})                                       \
    AST_KIND(TypeString, {})                                      \
    AST_KIND(TypeBool, {})                                        \
    AST_KIND(TypeVoid, {})                                        \
                                                                  \
    AST_KIND(TypeProcedure, {                                     \
        AstTypeArray Parameters;                                  \
        AstType* ReturnType;                                      \
    })                                                            \
                                                                  \
    AST_KIND_END(Type)

typedef enum AstKind {
#define AST_KIND(name, data)       AstKind_##name,
#define AST_KIND_BEGIN(name, data) AstKind_Begin_##name,
#define AST_KIND_END(name)         AstKind_End_##name,
    AST_KINDS
#undef AST_KIND
#undef AST_KIND_BEGIN
#undef AST_KIND_END
} AstKind;

#define AST_KIND(name, data)       +1
#define AST_KIND_BEGIN(name, data) +1
#define AST_KIND_END(name)         +1
enum { AstKind_Count = 0 AST_KINDS };
#undef AST_KIND
#undef AST_KIND_BEGIN
#undef AST_KIND_END

extern String AstKind_Names[AstKind_Count];

typedef struct Ast Ast;

#define AST_KIND(name, data)       typedef Ast Ast##name;
#define AST_KIND_BEGIN(name, data) typedef Ast Ast##name;
#define AST_KIND_END(name)
AST_KINDS
#undef AST_KIND
#undef AST_KIND_BEGIN
#undef AST_KIND_END

ARRAY_DECL(AstStatement*, AstStatement);
ARRAY_DECL(AstDeclaration*, AstDeclaration);
ARRAY_DECL(AstExpression*, AstExpression);
ARRAY_DECL(AstType*, AstType);
ARRAY_DECL(AstProcedure*, AstProcedure);

#define AST_KIND(name, data) typedef struct Ast##name##Data data Ast##name##Data;
#define AST_KIND_BEGIN(name, data)
#define AST_KIND_END(name)
AST_KINDS
#undef AST_KIND
#undef AST_KIND_BEGIN
#undef AST_KIND_END

struct Ast {
    AstKind Kind;
    uint64_t ID;
    union {
#define AST_KIND(name, data) Ast##name##Data name;
#define AST_KIND_BEGIN(name, data) \
    struct {                       \
        struct data name;          \
        union {
#define AST_KIND_END(name) \
    }                      \
    ;                      \
    }                      \
    ;
        AST_KINDS
#undef AST_KIND
#undef AST_KIND_BEGIN
#undef AST_KIND_END
    };
};

#define AST_KIND(name, data) Ast##name* Ast##name##_Create();
#define AST_KIND_BEGIN(name, data)
#define AST_KIND_END(name)
AST_KINDS
#undef AST_KIND
#undef AST_KIND_BEGIN
#undef AST_KIND_END

#define AST_KIND(name, data)
#define AST_KIND_BEGIN(name, data) bool Ast_Is##name(Ast* ast);
#define AST_KIND_END(name)
AST_KINDS
#undef AST_KIND
#undef AST_KIND_BEGIN
#undef AST_KIND_END

void Ast_Print(Ast* ast, uint64_t indent);
