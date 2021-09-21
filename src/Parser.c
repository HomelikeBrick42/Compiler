#include "Parser.h"

#include <stdlib.h>
#include <stdio.h>

bool Parser_Create(Parser* parser, const char* filePath) {
    if (!parser) {
        return false;
    }

    *parser = (Parser){};

    if (!Lexer_Create(&parser->Lexer, filePath)) {
        return false;
    }

    parser->Previous = (Token){
        .Kind     = TokenKind_EndOfFile,
        .FilePath = parser->Lexer.FilePath,
        .Source   = parser->Lexer.Source,
        .Position = 0,
        .Line     = 1,
        .Column   = 0,
        .Length   = 0,
    };

    parser->Current = Lexer_NextToken(&parser->Lexer);

    return true;
}

void Parser_Destroy(Parser* parser) {
    Lexer_Destroy(&parser->Lexer);
}

Token Parser_NextToken(Parser* parser) {
    parser->Previous = parser->Current;
    parser->Current  = Lexer_NextToken(&parser->Lexer);
    return parser->Previous;
}

Token Parser_ExpectToken(Parser* parser, TokenKind expected) {
    if (parser->Current.Kind != expected) {
        Token token = Parser_NextToken(parser);
        fflush(stdout);
        fprintf(stderr,
                "%s:%llu:%llu Expected token '%s', got token '%s'\n",
                token.FilePath,
                token.Line,
                token.Column,
                TokenKind_Names[expected],
                TokenKind_Names[token.Kind]);
        token.Kind       = expected;
        parser->WasError = true;
        return token;
    } else {
        return Parser_NextToken(parser);
    }
}

AstStatement* Parser_ParseStatement(Parser* parser) {
    AstExpression* expression = Parser_ParseExpression(parser);
    if (expression->Kind == AstKind_Name && parser->Current.Kind == TokenKind_Colon) {
        Token name = expression->Name.Token;
        free(expression);
        AstDeclaration* declaration = Parser_ParseDeclaration(parser, name);
        Parser_ExpectToken(parser, TokenKind_Semicolon);
        return declaration;
    } else {
        Parser_ExpectToken(parser, TokenKind_Semicolon);
        return expression;
    }
}

AstDeclaration* Parser_ParseDeclaration(Parser* parser, Token name) {
    AstDeclaration* declaration = calloc(1, sizeof(AstDeclaration));
    declaration->Kind           = AstKind_Declaration;

    declaration->Declaration.Name = name;

    Parser_ExpectToken(parser, TokenKind_Colon);
    declaration->Declaration.Type = Parser_ParseExpression(parser);

    if (parser->Current.Kind == TokenKind_Equals) {
        declaration->Declaration.EqualsToken = Parser_ExpectToken(parser, TokenKind_Equals);
        declaration->Declaration.Value       = Parser_ParseExpression(parser);
    }

    return declaration;
}

AstExpression* Parser_ParseExpression(Parser* parser) {
    return Parser_ParseBinaryExpression(parser, 0);
}

AstExpression* Parser_ParsePrimaryExpression(Parser* parser) {
    switch (parser->Current.Kind) {
        case TokenKind_Integer: {
            AstInteger* integer    = calloc(1, sizeof(AstInteger));
            integer->Kind          = AstKind_Integer;
            integer->Integer.Token = Parser_ExpectToken(parser, TokenKind_Integer);
            return integer;
        } break;

        case TokenKind_Identifier: {
            AstName* name    = calloc(1, sizeof(AstName));
            name->Kind       = AstKind_Name;
            name->Name.Token = Parser_ExpectToken(parser, TokenKind_Identifier);
            return name;
        } break;

        default: {
            fflush(stdout);
            fprintf(stderr,
                    "%s:%llu:%llu Unexpected token '%s' in expression\n",
                    parser->Current.FilePath,
                    parser->Current.Line,
                    parser->Current.Column,
                    TokenKind_Names[parser->Current.Kind]);
            parser->WasError = true;
            return NULL;
        } break;
    }
}

uint64_t Parser_GetUnaryOperatorPrecedence(TokenKind kind) {
    switch (kind) {
        case TokenKind_Plus:
        case TokenKind_Minus:
        case TokenKind_Bang:
            return 4;

        default:
            return 0;
    }
}

uint64_t Parser_GetBinaryOperatorPrecedence(TokenKind kind) {
    switch (kind) {
        case TokenKind_Asterisk:
        case TokenKind_Slash:
            return 3;

        case TokenKind_Plus:
        case TokenKind_Minus:
            return 2;

        case TokenKind_EqualsEquals:
        case TokenKind_BangEquals:
            return 1;

        default:
            return 0;
    }
}

AstExpression* Parser_ParseBinaryExpression(Parser* parser, uint64_t parentPrecedence) {
    AstExpression* left;

    uint64_t unaryPrecedence = Parser_GetUnaryOperatorPrecedence(parser->Current.Kind);
    if (unaryPrecedence != 0) {
        AstUnary* unary = calloc(1, sizeof(AstUnary));
        unary->Kind     = AstKind_Unary;

        unary->Unary.Operator = Parser_NextToken(parser);
        unary->Unary.Operand  = Parser_ParseBinaryExpression(parser, unaryPrecedence);

        left = unary;
    } else {
        left = Parser_ParsePrimaryExpression(parser);
    }

    while (true) {
        uint64_t binaryPrecedence = Parser_GetBinaryOperatorPrecedence(parser->Current.Kind);
        if (binaryPrecedence <= parentPrecedence) {
            break;
        }

        AstBinary* binary = calloc(1, sizeof(AstBinary));
        binary->Kind      = AstKind_Binary;

        binary->Binary.Left     = left;
        binary->Binary.Operator = Parser_NextToken(parser);
        binary->Binary.Right    = Parser_ParseBinaryExpression(parser, binaryPrecedence);

        left = binary;
    }

    return left;
}
