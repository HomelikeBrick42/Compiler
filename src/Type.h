#pragma once

#include "Ast.h"

#include <stdint.h>
#include <stdbool.h>

#define TYPE_KINDS                       \
    TYPE_KIND(Type, {})                  \
                                         \
    TYPE_KIND(Integer, { bool Signed; }) \
    TYPE_KIND(Float, {})                 \
    TYPE_KIND(Bool, {})                  \
                                         \
    TYPE_KIND(Procedure, {               \
        Type** ParameterTypes;           \
        uint64_t ParameterTypeCount;     \
        Type* ReturnType;                \
    })

typedef enum TypeKind {
#define TYPE_KIND(name, data) TypeKind_##name,
    TYPE_KINDS
#undef TYPE_KIND
} TypeKind;

#define TYPE_KIND(name, data) +1
enum { TypeKind_Count = 0 TYPE_KINDS };
#undef TYPE_KIND

extern const char* TypeKind_Names[TypeKind_Count];

typedef struct Type Type;

#define TYPE_KIND(name, data) typedef struct Type Type##name;
TYPE_KINDS
#undef TYPE_KIND

#define TYPE_KIND(name, data) typedef struct data Type##name##Data;
TYPE_KINDS
#undef TYPE_KIND

struct Type {
    TypeKind Kind;
    uint64_t Size;
    union {
#define TYPE_KIND(name, data) Type##name##Data name;
        TYPE_KINDS
#undef TYPE_KIND
    };
};

extern const char* Type_TypeName;
extern Type Type_Type;
extern AstTypeExpression Type_TypeExpression;
extern AstDeclaration Type_TypeDeclaration;

extern const char* Type_IntegerSignedName;
extern Type Type_IntegerSigned;
extern AstTypeExpression Type_IntegerSignedExpression;
extern AstDeclaration Type_IntegerSignedDeclaration;

extern const char* Type_IntegerUnsignedName;
extern Type Type_IntegerUnsigned;
extern AstTypeExpression Type_IntegerUnsignedExpression;
extern AstDeclaration Type_IntegerUnsignedDeclaration;

extern const char* Type_BoolName;
extern Type Type_Bool;
extern AstTypeExpression Type_BoolExpression;
extern AstDeclaration Type_BoolDeclaration;

void InitTypes();
bool TypesEqual(Type* a, Type* b);
