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

AstScope* Parser_ParseFile(Parser* parser) {
    AstScope* scope     = calloc(1, sizeof(AstScope));
    scope->Kind         = AstKind_Scope;
    scope->Scope.Global = true;

    parser->ParentScope = scope;

    AstScope* parentFileScope = parser->ParentFileScope;
    parser->ParentFileScope   = scope;

    while (parser->Current.Kind != TokenKind_EndOfFile) {
        scope->Scope.Statements = realloc(scope->Scope.Statements, (scope->Scope.StatementCount + 1) * sizeof(AstStatement*));
        scope->Scope.Statements[scope->Scope.StatementCount] = Parser_ParseStatement(parser);
        scope->Scope.StatementCount++;
    }

    parser->ParentFileScope = parentFileScope;
    parser->ParentScope     = scope->Scope.ParentScope;

    return scope;
}

AstStatement* Parser_ParseStatement(Parser* parser) {
    while (parser->Current.Kind == TokenKind_Semicolon) {
        Parser_ExpectToken(parser, TokenKind_Semicolon);
    }

    switch (parser->Current.Kind) {
        case TokenKind_KeywordIf: {
            AstIf* iff = calloc(1, sizeof(AstIf));
            iff->Kind  = AstKind_If;

            Parser_ExpectToken(parser, TokenKind_KeywordIf);

            iff->If.Condition = Parser_ParseExpression(parser);
            if (parser->Current.Kind != TokenKind_OpenBrace) {
                Parser_ExpectToken(parser, TokenKind_KeywordDo);
                iff->If.ThenStatement = Parser_ParseStatement(parser);
            } else {
                iff->If.ThenStatement = Parser_ParseScope(parser, true);
            }

            if (parser->Current.Kind == TokenKind_KeywordElse) {
                Parser_ExpectToken(parser, TokenKind_KeywordElse);
                iff->If.ElseStatement = Parser_ParseStatement(parser);
            }

            return iff;
        } break;

        case TokenKind_OpenBrace: {
            return Parser_ParseScope(parser, true);
        } break;

        case TokenKind_KeywordWhile: {
            AstWhile* whilee = calloc(1, sizeof(AstWhile));
            whilee->Kind     = AstKind_While;

            Parser_ExpectToken(parser, TokenKind_KeywordWhile);

            whilee->While.Condition = Parser_ParseExpression(parser);
            whilee->While.Scope     = Parser_ParseScope(parser, true);

            return whilee;
        } break;

        case TokenKind_KeywordBreak: {
            AstBreak* breakk = calloc(1, sizeof(AstBreak));
            breakk->Kind     = AstKind_Break;
            Parser_ExpectToken(parser, TokenKind_KeywordBreak);
            Parser_ExpectToken(parser, TokenKind_Semicolon);
            return breakk;
        } break;

        case TokenKind_KeywordContinue: {
            AstContinue* continuee = calloc(1, sizeof(AstContinue));
            continuee->Kind        = AstKind_Continue;
            Parser_ExpectToken(parser, TokenKind_KeywordContinue);
            Parser_ExpectToken(parser, TokenKind_Semicolon);
            return continuee;
        } break;

        case TokenKind_KeywordPrint: {
            AstPrint* print = calloc(1, sizeof(AstPrint));

            print->Kind = AstKind_Print;
            Parser_ExpectToken(parser, TokenKind_KeywordPrint);
            print->Print.Value = Parser_ParseExpression(parser);
            Parser_ExpectToken(parser, TokenKind_Semicolon);

            return print;
        } break;

        case TokenKind_KeywordReturn: {
            AstReturn* ret = calloc(1, sizeof(AstReturn));
            ret->Kind      = AstKind_Return;

            Parser_ExpectToken(parser, TokenKind_KeywordReturn);
            if (parser->Current.Kind != TokenKind_Semicolon) {
                ret->Return.Value = Parser_ParseExpression(parser);
            }
            Parser_ExpectToken(parser, TokenKind_Semicolon);

            return ret;
        } break;

        default: {
            AstExpression* expression = Parser_ParseExpression(parser);
            if (expression && expression->Kind == AstKind_Name && parser->Current.Kind == TokenKind_Colon) {
                Token name = expression->Name.Token;
                free(expression);
                AstDeclaration* declaration = Parser_ParseDeclaration(parser, name, false);
                Parser_ExpectToken(parser, TokenKind_Semicolon);
                return declaration;
            } else if (parser->Current.Kind == TokenKind_Equals) {
                AstAssignment* assignment          = calloc(1, sizeof(AstAssignment));
                assignment->Kind                   = AstKind_Assignment;
                assignment->Assignment.Operand     = expression;
                assignment->Assignment.EqualsToken = Parser_ExpectToken(parser, TokenKind_Equals);
                assignment->Assignment.Value       = Parser_ParseExpression(parser);
                Parser_ExpectToken(parser, TokenKind_Semicolon);
                return assignment;
            } else {
                Parser_ExpectToken(parser, TokenKind_Semicolon);
                return expression;
            }
        } break;
    }
}

AstScope* Parser_ParseScope(Parser* parser, bool nested) {
    AstScope* scope          = calloc(1, sizeof(AstScope));
    scope->Kind              = AstKind_Scope;
    scope->Scope.ParentScope = parser->ParentScope;
    scope->Scope.Nested      = nested;

    parser->ParentScope = scope;

    Parser_ExpectToken(parser, TokenKind_OpenBrace);

    while (parser->Current.Kind != TokenKind_CloseBrace && parser->Current.Kind != TokenKind_EndOfFile) {
        scope->Scope.Statements = realloc(scope->Scope.Statements, (scope->Scope.StatementCount + 1) * sizeof(AstStatement*));
        scope->Scope.Statements[scope->Scope.StatementCount] = Parser_ParseStatement(parser);
        scope->Scope.StatementCount++;
    }

    Parser_ExpectToken(parser, TokenKind_CloseBrace);

    parser->ParentScope = scope->Scope.ParentScope;

    return scope;
}

AstDeclaration* Parser_ParseDeclaration(Parser* parser, Token name, bool isProcedureParam) {
    AstDeclaration* declaration               = calloc(1, sizeof(AstDeclaration));
    declaration->Kind                         = AstKind_Declaration;
    declaration->Declaration.Constant         = false;
    declaration->Declaration.IsProcedureParam = isProcedureParam;

    declaration->Declaration.Name = name;

    Parser_ExpectToken(parser, TokenKind_Colon);

    if (parser->Current.Kind != TokenKind_Equals && parser->Current.Kind != TokenKind_Colon) {
        declaration->Declaration.Type = Parser_ParseExpression(parser);
    }

    if (parser->Current.Kind == TokenKind_Equals) {
        declaration->Declaration.EqualsToken = Parser_ExpectToken(parser, TokenKind_Equals);
        declaration->Declaration.Value       = Parser_ParseExpression(parser);
    } else if (parser->Current.Kind == TokenKind_Colon) {
        declaration->Declaration.EqualsToken = Parser_ExpectToken(parser, TokenKind_Colon);
        declaration->Declaration.Value       = Parser_ParseExpression(parser);
        declaration->Declaration.Constant    = true;
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

        case TokenKind_KeywordTrue: {
            Parser_ExpectToken(parser, TokenKind_KeywordTrue);
            AstTrue* truee = calloc(1, sizeof(AstTrue));
            truee->Kind    = AstKind_True;
            return truee;
        } break;

        case TokenKind_KeywordFalse: {
            Parser_ExpectToken(parser, TokenKind_KeywordFalse);
            AstFalse* falsee = calloc(1, sizeof(AstFalse));
            falsee->Kind     = AstKind_False;
            return falsee;
        } break;

        case TokenKind_OpenParenthesis: {
            Parser_ExpectToken(parser, TokenKind_OpenParenthesis);

            if (parser->Current.Kind == TokenKind_CloseParenthesis) {
                AstProcedure* procedure = calloc(1, sizeof(AstProcedure));
                procedure->Kind         = AstKind_Procedure;

                Parser_ExpectToken(parser, TokenKind_CloseParenthesis);

                if (parser->Current.Kind == TokenKind_RightArrow) {
                    Parser_ExpectToken(parser, TokenKind_RightArrow);
                    procedure->Procedure.ReturnType = Parser_ParseExpression(parser);
                }

                if (parser->Current.Kind == TokenKind_OpenBrace) {
                    AstScope* parentScope     = parser->ParentScope;
                    parser->ParentScope       = parser->ParentFileScope;
                    procedure->Procedure.Body = Parser_ParseScope(parser, false);
                    parser->ParentScope       = parentScope;
                }

                return procedure;
            }

            AstExpression* expression = Parser_ParseExpression(parser);

            if (expression && expression->Kind == AstKind_Name && parser->Current.Kind == TokenKind_Colon) {
                AstProcedure* procedure = calloc(1, sizeof(AstProcedure));
                procedure->Kind         = AstKind_Procedure;

                Token firstName = expression->Name.Token;
                free(expression);
                AstDeclaration* firstDeclaration = Parser_ParseDeclaration(parser, firstName, true);

                procedure->Procedure.Parameters     = malloc(1 * sizeof(AstDeclaration*));
                procedure->Procedure.Parameters[0]  = firstDeclaration;
                procedure->Procedure.ParameterCount = 1;

                while (parser->Current.Kind != TokenKind_CloseParenthesis && parser->Current.Kind != TokenKind_EndOfFile) {
                    Parser_ExpectToken(parser, TokenKind_Comma);
                    procedure->Procedure.Parameters = realloc(
                        procedure->Procedure.Parameters, (procedure->Procedure.ParameterCount + 1) * sizeof(AstDeclaration*));
                    Token name = Parser_ExpectToken(parser, TokenKind_Identifier);
                    procedure->Procedure.Parameters[procedure->Procedure.ParameterCount] =
                        Parser_ParseDeclaration(parser, name, true);
                    procedure->Procedure.ParameterCount++;
                }

                Parser_ExpectToken(parser, TokenKind_CloseParenthesis);

                if (parser->Current.Kind == TokenKind_RightArrow) {
                    Parser_ExpectToken(parser, TokenKind_RightArrow);
                    procedure->Procedure.ReturnType = Parser_ParseExpression(parser);
                }

                if (parser->Current.Kind == TokenKind_OpenBrace) {
                    AstScope* parentScope     = parser->ParentScope;
                    parser->ParentScope       = parser->ParentFileScope;
                    procedure->Procedure.Body = Parser_ParseScope(parser, false);
                    parser->ParentScope       = parentScope;
                }

                return procedure;
            } else {
                Parser_ExpectToken(parser, TokenKind_CloseParenthesis);
                return expression;
            }
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
        case TokenKind_KeywordTransmute:
        case TokenKind_KeywordCast:
            return 5;

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
        if (parser->Current.Kind == TokenKind_KeywordTransmute) {
            Parser_ExpectToken(parser, TokenKind_KeywordTransmute);

            AstTransmute* transmute = calloc(1, sizeof(AstTransmute));
            transmute->Kind         = AstKind_Transmute;

            Parser_ExpectToken(parser, TokenKind_OpenParenthesis);
            transmute->Transmute.Type = Parser_ParseExpression(parser);
            Parser_ExpectToken(parser, TokenKind_CloseParenthesis);

            transmute->Transmute.Expression = Parser_ParseBinaryExpression(parser, unaryPrecedence);
            left                            = transmute;
        } else if (parser->Current.Kind == TokenKind_KeywordCast) {
            Parser_ExpectToken(parser, TokenKind_KeywordCast);

            AstCast* cast = calloc(1, sizeof(AstCast));
            cast->Kind    = AstKind_Cast;

            Parser_ExpectToken(parser, TokenKind_OpenParenthesis);
            cast->Cast.Type = Parser_ParseExpression(parser);
            Parser_ExpectToken(parser, TokenKind_CloseParenthesis);

            cast->Cast.Expression = Parser_ParseBinaryExpression(parser, unaryPrecedence);
            left                  = cast;
        } else {
            AstUnary* unary = calloc(1, sizeof(AstUnary));
            unary->Kind     = AstKind_Unary;

            unary->Unary.Operator = Parser_NextToken(parser);
            unary->Unary.Operand  = Parser_ParseBinaryExpression(parser, unaryPrecedence);

            left = unary;
        }
    } else {
        left = Parser_ParsePrimaryExpression(parser);
    }

    while (true) {
        if (parser->Current.Kind == TokenKind_OpenParenthesis) {
            AstCall* call      = calloc(1, sizeof(AstCall));
            call->Kind         = AstKind_Call;
            call->Call.Operand = left;

            Parser_ExpectToken(parser, TokenKind_OpenParenthesis);

            while (parser->Current.Kind != TokenKind_CloseParenthesis && parser->Current.Kind != TokenKind_EndOfFile) {
                call->Call.Arguments = realloc(call->Call.Arguments, (call->Call.ArgumentCount + 1) * sizeof(AstExpression*));
                call->Call.Arguments[call->Call.ArgumentCount] = Parser_ParseExpression(parser);
                call->Call.ArgumentCount++;
                if (parser->Current.Kind != TokenKind_CloseParenthesis) {
                    Parser_ExpectToken(parser, TokenKind_Comma);
                }
            }

            Parser_ExpectToken(parser, TokenKind_CloseParenthesis);

            left = call;
        } else {
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
    }

    return left;
}
