#pragma once

#define AST_KIND(name, data)
#define AST_KIND_BEGIN(name, data) case AstKind_Begin_##name:
#define AST_KIND_END(name)         case AstKind_End_##name:
AST_KINDS {}
break;
#undef AST_KIND
#undef AST_KIND_BEGIN
#undef AST_KIND_END
