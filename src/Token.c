#include "Token.h"

const char* TokenKind_Names[TokenKind_Count] = {
#define TOKEN_KIND(name, str) [TokenKind_##name] = str,
    TOKEN_KINDS
#undef TOKEN_KIND
};
