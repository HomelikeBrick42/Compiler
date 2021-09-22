#pragma once

#include "Lexer.h"
#include "Ast.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct Parser {
    Lexer Lexer;
    Token Current;
    Token Previous;
    bool WasError;
    AstScope* ParentFileScope;
    AstScope* ParentScope;
} Parser;

bool Parser_Create(Parser* parser, const char* filePath);
void Parser_Destroy(Parser* parser);

Token Parser_NextToken(Parser* parser);
Token Parser_ExpectToken(Parser* parser, TokenKind expected);

AstScope* Parser_ParseFile(Parser* parser);

AstStatement* Parser_ParseStatement(Parser* parser);
AstScope* Parser_ParseScope(Parser* parser);
AstDeclaration* Parser_ParseDeclaration(Parser* parser, Token name, bool isProcedureParam);

AstExpression* Parser_ParseExpression(Parser* parser);
AstExpression* Parser_ParsePrimaryExpression(Parser* parser);
uint64_t Parser_GetUnaryOperatorPrecedence(TokenKind kind);
uint64_t Parser_GetBinaryOperatorPrecedence(TokenKind kind);
AstExpression* Parser_ParseBinaryExpression(Parser* parser, uint64_t parentPrecedence);
