#pragma once

#include "Token.h"
#include "Lexer.h"
#include "Ast.h"

#include <stdint.h>

typedef struct Parser {
    Lexer Lexer;
    Token PreviousToken;
    Token Current;
    AstScope* ParentScope;
    bool Error;
} Parser;

Parser Parser_Create(String filePath, String source);

Token Parser_NextToken(Parser* parser);
Token Parser_ExpectToken(Parser* parser, TokenKind kind);

AstScope* Parser_Parse(Parser* parser);

AstStatement* Parser_ParseStatement(Parser* parser);
AstStatement* Parser_ParseScope(Parser* parser, AstProcedure* parentProcedure);
AstDeclaration* Parser_ParseDeclaration(Parser* parser, Token name, AstProcedure* parentProcedure);

AstExpression* Parser_ParseExpression(Parser* parser);
AstExpression* Parser_ParsePrimaryExpression(Parser* parser);
uint64_t Parser_GetUnaryOperatorPrecedence(TokenKind kind);
uint64_t Parser_GetBinaryOperatorPrecedence(TokenKind kind);
AstExpression* Parser_ParseBinaryExpression(Parser* parser, uint64_t parentPrecedence);

inline bool Parser_WasError(Parser* parser) {
    return parser->Error || parser->Lexer.Error;
}
