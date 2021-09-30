#pragma once

#define TOKEN_KIND(name, str)
#define TOKEN_KIND_BEGIN(name) case TokenKind_Begin_##name:
#define TOKEN_KIND_END(name)   case TokenKind_End_##name:
TOKEN_KINDS {}
break;
#undef TOKEN_KIND
#undef TOKEN_KIND_BEGIN
#undef TOKEN_KIND_END
