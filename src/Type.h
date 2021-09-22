#pragma once

#include <stdint.h>
#include <stdbool.h>

#define TYPE_KINDS                       \
    TYPE_KIND(Type, {})                  \
    TYPE_KIND(Integer, { bool Signed; }) \
    TYPE_KIND(Float, {})

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
    union {
#define TYPE_KIND(name, data) Type##name##Data name;
        TYPE_KINDS
#undef TYPE_KIND
    };
};
