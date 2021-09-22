#include "Type.h"

const char* TypeKind_Names[TypeKind_Count] = {
#define TYPE_KIND(name, data) [TypeKind_##name] = #name,
    TYPE_KINDS
#undef TYPE_KIND
};
