#pragma once

#include "Ast.h"
#include "Array.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct UnaryOperator {
    TokenKind Operator;
    AstType* Operand;
    AstType* Result;
} UnaryOperator;

typedef struct BinaryOperator {
    TokenKind Operator;
    AstType* Left;
    AstType* Right;
    AstType* Result;
} BinaryOperator;

typedef struct Cast {
    AstType* From;
    AstType* To;
} Cast;

ARRAY_DECL(UnaryOperator, UnaryOperator);
ARRAY_DECL(BinaryOperator, BinaryOperator);
ARRAY_DECL(Cast, Cast);

typedef struct Resolver {
    AstScope* GlobalScope;
    // TODO: Move these to another structure
    AstTypeArray InternedTypes;
    UnaryOperatorArray UnaryOperators;
    BinaryOperatorArray BinaryOperators;
    CastArray Casts;
    AstProcedureArray PendingProcedures;
} Resolver;

Resolver Resolver_Create(AstScope* globalScope);
void Resolver_Destroy(Resolver* resolver);

bool Resolver_Resolve(Resolver* resolver);
bool Resolver_ResolveAst(Resolver* resolver, Ast* ast, AstType* expectedType);

AstType* Resolver_ExpressionToType(Resolver* resolver, AstExpression* expression);
AstTypeType* Resolver_CreateTypeType(Resolver* resolver);
AstTypeBool* Resolver_CreateTypeBool(Resolver* resolver);
AstTypeInteger* Resolver_CreateTypeInteger(Resolver* resolver, uint64_t size, bool signedd);
AstTypeFloat* Resolver_CreateTypeFloat(Resolver* resolver, uint64_t size);
AstTypeString* Resolver_CreateTypeString(Resolver* resolver);
AstTypeString* Resolver_CreateTypeVoid(Resolver* resolver);
AstTypeProcedure* Resolver_CreateTypeProcedure(Resolver* resolver, AstDeclarationArray parameters, AstType* returnType);
