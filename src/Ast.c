#include "Ast.h"

const char* AstKind_Names[AstKind_Count] = {
#define AST_KIND_BEGIN(name)
#define AST_KIND_END(name)
#define AST_KIND(name, data) [AstKind_##name] = #name,
    AST_KINDS
#undef AST_KIND
#undef AST_KIND_END
#undef AST_KIND_BEGIN
};
